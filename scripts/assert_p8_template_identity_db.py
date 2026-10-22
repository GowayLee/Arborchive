#!/usr/bin/env python3
"""Check instantiated inner aggregates, not deferred generic-template root identity."""
import argparse
from collections import Counter
import hashlib
from pathlib import Path
import sqlite3
import tempfile

SOURCE = Path(__file__).resolve().parents[1] / 'tests/unit-tests/p8/aggregate_template_identity_case.cc'


def validate(c):
    errors = []
    source = SOURCE.read_text().splitlines()
    lines = {name: next(i for i, text in enumerate(source, 1) if marker in text)
             for name, marker in [('x', '// FIELD:x'), ('g', '// FIELD:g'), ('init', '// INIT')]}
    fields = {}
    for name in ('x', 'g'):
        rows = c.execute('''select distinct m.id from membervariables m
            join var_decls d on d.variable=m.id
            join locations_default l on l.id=d.location
            where m.name=? and l.start_line=?''', (name, lines[name])).fetchall()
        if len(rows) != 1:
            errors.append('field declaration: ' + name)
        else:
            fields[name] = rows[0][0]
    if errors:
        return errors
    bindings = {}
    for name in ('x', 'g'):
        rows = c.execute('select aggregate,initializer,position from aggregate_field_init where field=?',
                         (fields[name],)).fetchall()
        if len(rows) != 1:
            errors.append('exact inner binding: ' + name)
        else:
            bindings[name] = rows[0]
    if errors:
        return errors
    outer, child, outer_pos = bindings['g']
    inner, leaf, inner_pos = bindings['x']
    if len({outer, inner, leaf}) != 3 or child != inner or (outer_pos, inner_pos) != (0, 0):
        errors.append('distinct inner aggregate/leaf chain')
    expected_fields = [(outer, inner, fields['g'], 0), (inner, leaf, fields['x'], 0)]
    if Counter(c.execute('select * from aggregate_field_init').fetchall()) != Counter(expected_fields):
        errors.append('exact complete inner field bindings')
    for node in (outer, inner):
        rows = c.execute('''select e.kind,l.start_line from exprs e
            join locations_expr l on l.id=e.location where e.id=?''', (node,)).fetchall()
        if rows != [(91, lines['init'])]:
            errors.append('inner aggregate source/kind')
    literal = c.execute('''select e.kind,v.str from exprs e join valuebind b on b.expr=e.id
        join "values" v on v.id=b.val where e.id=?''', (leaf,)).fetchall()
    if literal != [(123, '555')]:
        errors.append('reachable inner literal')
    edges = c.execute('select * from exprparents where parent_id in (?,?)', (outer, inner)).fetchall()
    if Counter(edges) != Counter([(inner, 0, outer), (leaf, 0, inner)]):
        errors.append('exact inner parent edges')
    if c.execute('pragma integrity_check').fetchall() != [('ok',)]:
        errors.append('database integrity')
    return errors


def mutations(database):
    before = hashlib.sha256(database.read_bytes()).hexdigest()
    changes = {
        'collapse_inner': 'update aggregate_field_init set initializer=aggregate',
        'missing_inner': 'delete from exprs where id in (select aggregate from aggregate_field_init)',
        'missing_leaf_parent': 'delete from exprparents where expr_id in (select id from exprs where kind=123)',
        'extra_inner_parent': '''insert into exprparents select initializer,9,aggregate from aggregate_field_init limit 1''',
        'wrong_field': 'update aggregate_field_init set field=(select min(field) from aggregate_field_init)',
        'wrong_literal': 'update "values" set str=\'CORRUPTED\'',
    }
    with sqlite3.connect(f'file:{database.resolve()}?mode=ro', uri=True) as original:
        with tempfile.TemporaryDirectory(prefix='arborchive-p8-template-') as folder:
            for label, sql in changes.items():
                with sqlite3.connect(str(Path(folder)/(label+'.db'))) as copy:
                    original.backup(copy)
                    copy.executescript(sql)
                    if not validate(copy):
                        raise AssertionError('mutation escaped: ' + label)
    assert hashlib.sha256(database.read_bytes()).hexdigest() == before, 'source changed'
    print(f'P8_TEMPLATE_IDENTITY_MUTATIONS_OK cases={len(changes)} source_unchanged=yes')


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
            print('P8_TEMPLATE_IDENTITY_FAIL ' + error)
        return 1
    print('P8_TEMPLATE_IDENTITY_OK cases=1 inner_aggregates=2 fields=2 exact_subgraph=yes generic_root=deferred')
    if args.mutation_tests:
        mutations(args.database)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
