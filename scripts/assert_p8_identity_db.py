#!/usr/bin/env python3
"""Exact source-anchored initialization graphs for shadowed aggregate types."""
import argparse
from collections import Counter
import hashlib
from pathlib import Path
import re
import sqlite3
import tempfile

SOURCE = Path(__file__).resolve().parents[1] / 'tests/unit-tests/p8/aggregate_identity_case.cc'
# Each step consumes child zero. None denotes an array element rather than a field.
CASES = {
    'shadow': (['shadow_s', 'shadow_g', 'global_x'], '111'),
    'block': (['block_s', 'block_g', 'block_x'], '222'),
    'arrays': (['array_s', None, 'array_g', None, 'global_x'], '333'),
    'forward': (['forward_s', 'forward_g', 'forward_x'], '444'),
}


def markers():
    result = {}
    for line, text in enumerate(SOURCE.read_text().splitlines(), 1):
        match = re.search(r'// (FIELD|CASE):(\w+)', text)
        if match:
            key = match.group(2)
            if key in result:
                raise ValueError('duplicate source marker: ' + key)
            result[key] = line
    return result


def validate(c):
    errors, fields, arrays, edges, roots = [], [], [], [], []
    lines = markers()

    def require(condition, message):
        if not condition:
            errors.append(message)
        return condition

    for name, (steps, literal) in CASES.items():
        declarations = c.execute('''select distinct d.variable from var_decls d
            join locations_default l on l.id=d.location
            where d.name=? and l.start_line=?''', (name, lines[name])).fetchall()
        if not require(len(declarations) == 1, name + ': variable declaration'):
            continue
        initialisers = c.execute('select init,expr from initialisers where var=?',
                                 declarations[0]).fetchall()
        if not require(len(initialisers) == 1, name + ': initialiser count'):
            continue
        init, node = initialisers[0]
        roots.append(init)
        edges.append((node, 0, init))
        seen = []
        for depth, field in enumerate(steps):
            if not require(node not in seen, name + ': collapsed aggregate identities'):
                break
            seen.append(node)
            expr = c.execute('''select e.kind,l.start_line from exprs e
                join locations_expr l on l.id=e.location where e.id=?''', (node,)).fetchall()
            require(expr == [(91, lines[name])], name + ': aggregate kind/source')
            if field is None:
                rows = c.execute('''select initializer,element_index,position
                    from aggregate_array_init where aggregate=?''', (node,)).fetchall()
                if not require(len(rows) == 1, name + ': array child count'):
                    break
                child, element, position = rows[0]
                require((element, position) == (0, 0), name + ': array index/position')
                arrays.append((node, child, 0, 0))
            else:
                ids = c.execute('''select distinct m.id from membervariables m
                    join var_decls d on d.variable=m.id
                    join locations_default l on l.id=d.location
                    where l.start_line=?''', (lines[field],)).fetchall()
                if not require(len(ids) == 1, name + ': field declaration ' + field):
                    break
                rows = c.execute('''select initializer,field,position
                    from aggregate_field_init where aggregate=?''', (node,)).fetchall()
                if not require(len(rows) == 1, name + ': field child count'):
                    break
                child, actual_field, position = rows[0]
                require((actual_field, position) == (ids[0][0], 0),
                        name + ': field identity/position ' + field)
                fields.append((node, child, ids[0][0], 0))
            edges.append((child, 0, node))
            node = child
        else:
            require(node not in seen, name + ': leaf aliases aggregate')
            leaves = c.execute('''select e.kind,v.str from exprs e
                join valuebind b on b.expr=e.id join "values" v on v.id=b.val
                where e.id=?''', (node,)).fetchall()
            require(leaves == [(123, literal)], name + ': reachable literal')
        actual_nodes = c.execute('''select e.id from exprs e join locations_expr l
            on l.id=e.location where e.kind=91 and l.start_line=?''', (lines[name],)).fetchall()
        require(len(seen) == len(steps) and {r[0] for r in actual_nodes} == set(seen),
                name + ': complete distinct aggregate node set')

    for table, expected in [('aggregate_field_init', fields),
                            ('aggregate_array_init', arrays), ('exprparents', edges)]:
        require(Counter(c.execute('select * from ' + table).fetchall()) == Counter(expected),
                'exact complete ' + table)
    require(Counter(c.execute('select init from initialisers').fetchall()) == Counter((x,) for x in roots),
            'exact initialiser roots')
    require(len(roots) == len(CASES), 'all expected roots')
    require(Counter(c.execute('select * from braced_initialisers').fetchall()) == Counter((x,) for x in roots),
            'exact braced roots')
    require(c.execute('pragma integrity_check').fetchall() == [('ok',)], 'database integrity')
    return errors


def mutations(database):
    before = hashlib.sha256(database.read_bytes()).hexdigest()

    def collapse_shadow(c):
        root = c.execute('''select i.expr from initialisers i
            join localvariables v on v.id=i.var where v.name='shadow' ''').fetchone()[0]
        outer = c.execute('select initializer from aggregate_field_init where aggregate=?',
                          (root,)).fetchone()[0]
        inner = c.execute('select initializer from aggregate_field_init where aggregate=?',
                          (outer,)).fetchone()[0]
        c.execute('delete from aggregate_field_init where aggregate=?', (inner,))
        c.execute('update aggregate_field_init set initializer=? where aggregate=?', (outer, outer))
        c.execute('delete from exprparents where parent_id in (?,?)', (outer, inner))
        c.execute('insert into exprparents values (?,0,?)', (outer, outer))
        c.execute('delete from exprs where id=?', (inner,))

    changes = {
        'collapsed_shadow_identity': collapse_shadow,
        'missing_initialisers': 'delete from initialisers',
        'wrong_root': 'update initialisers set expr=-1',
        'missing_braced': 'delete from braced_initialisers',
        'missing_root_parent': 'delete from exprparents where parent_id in (select init from initialisers)',
        'extra_parent': '''insert into exprparents select expr_id,99,parent_id from exprparents limit 1''',
        'disconnected_leaf': '''delete from exprparents where expr_id in (select id from exprs where kind=123)''',
        'self_initializer': 'update aggregate_field_init set initializer=aggregate',
        'missing_field': 'delete from aggregate_field_init',
        'wrong_field': 'update aggregate_field_init set field=(select min(id) from membervariables)',
        'wrong_array_position': 'update aggregate_array_init set position=9',
        'missing_aggregate': 'delete from exprs where kind=91',
        'wrong_literal': 'update "values" set str=\'CORRUPTED\'',
    }
    with sqlite3.connect(f'file:{database.resolve()}?mode=ro', uri=True) as original:
        with tempfile.TemporaryDirectory(prefix='arborchive-p8-identity-') as folder:
            for label, sql in changes.items():
                with sqlite3.connect(str(Path(folder)/(label+'.db'))) as copy:
                    original.backup(copy)
                    if callable(sql):
                        sql(copy)
                    else:
                        copy.executescript(sql)
                    if not validate(copy):
                        raise AssertionError('mutation escaped: ' + label)
    assert hashlib.sha256(database.read_bytes()).hexdigest() == before, 'source changed'
    print(f'P8_IDENTITY_MUTATIONS_OK cases={len(changes)} source_unchanged=yes')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('database', type=Path)
    parser.add_argument('--mutation-tests', action='store_true')
    args = parser.parse_args()
    if not args.database.is_file():
        parser.error('database does not exist')
    with sqlite3.connect(f'file:{args.database.resolve()}?mode=ro', uri=True) as c:
        errors = validate(c)
    if errors:
        for error in errors:
            print('P8_IDENTITY_FAIL ' + error)
        return 1
    print('P8_IDENTITY_OK cases=4 aggregates=14 fields=12 arrays=2 roots=4 exact_graph=yes')
    if args.mutation_tests:
        mutations(args.database)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
