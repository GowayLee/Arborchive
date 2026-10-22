#!/usr/bin/env python3
"""Verify aggregate field/value/position identities, including corrupt-copy controls."""
import argparse
from collections import Counter
import hashlib
from pathlib import Path
import re
import sqlite3
import tempfile

SOURCE = Path(__file__).resolve().parents[1] / 'tests/unit-tests/p8/aggregate_binding_case.cc'
# (aggregate type, field, semantic initializer position, initializer kind, value/type)
CASES = {
 'derived': [('P8Base','base_value',0,123,'11'),('P8Derived','own_value',1,123,'22')],
 'multi': [('P8Left','left_value',0,123,'31'),('P8Right','right_value',0,123,'32'),('P8Multi','multi_value',2,123,'33')],
 'union_first': [('P8Choice','first',0,123,'41')],
 'union_second': [('P8Choice','second',0,123,'42')],
 'nested': [('P8Nested','pair',0,91,'P8Pair'),('P8Nested','tail',1,123,'53'),('P8Pair','x',0,123,'51'),('P8Pair','y',1,123,'52')],
 'elided': [('P8Nested','pair',0,91,'P8Pair'),('P8Nested','tail',1,123,'63'),('P8Pair','x',0,123,'61'),('P8Pair','y',1,123,'62')],
 'ordinary': [('P8Pair','x',0,123,'71'),('P8Pair','y',1,123,'72')],
 'bits': [('P8Bits','before',0,123,'81'),('P8Bits','after',1,123,'82')],
 'elided_base': [('P8Base','base_value',0,123,'91'),('P8Derived','own_value',1,123,'92')],
 'deep': [('P8Chain','layer',0,91,'P8Layer'),('P8Layer','base',0,91,'P8Base'),('P8Base','base_value',0,123,'101')],
}
# (derived aggregate type, semantic child index, base aggregate type)
BASE_EDGES = {
 'derived': [('P8Derived', 0, 'P8Base')],
 'multi': [('P8Multi', 0, 'P8Left'), ('P8Multi', 1, 'P8Right')],
 'elided_base': [('P8Derived', 0, 'P8Base')],
}


def fixture_lines():
    source = SOURCE.read_text().splitlines()
    result = {}
    for name in CASES:
        lines = [i for i,s in enumerate(source,1) if re.search(r'\b'+name+r'\s*\{', s)]
        if len(lines) != 1:
            raise ValueError('fixture marker is ambiguous: ' + name)
        result[name] = lines[0]
    return result


def expected_base_edges(connection, lines):
    edges, errors = [], []
    for name, facts in BASE_EDGES.items():
        for parent_type, index, child_type in facts:
            ids = []
            for type_name in (parent_type, child_type):
                rows = connection.execute('''select e.id,t.typeid from exprs e
                    join locations_expr l on l.id=e.location
                    join expr_types t on t.id=e.id join usertypes u on u.id=t.typeid
                    where e.kind=91 and l.start_line=? and u.name=?''',
                    (lines[name], type_name)).fetchall()
                if len(rows) != 1:
                    errors.append('base aggregate identity: '+repr((name,type_name,rows)))
                    break
                ids.append(rows[0][0])
            if len(ids) == 2:
                edges.append((ids[1], index, ids[0]))
    return edges, errors


def delete_base_edges(connection):
    edges, errors = expected_base_edges(connection, fixture_lines())
    if errors:
        raise AssertionError('base edge mutation setup: '+repr(errors))
    before = connection.total_changes
    connection.executemany('''delete from exprparents
        where expr_id=? and child_index=? and parent_id=?''', edges)
    if connection.total_changes - before != 4:
        raise AssertionError('base edge mutation must remove exactly four edges')


def validate(connection):
    errors = []
    lines = fixture_lines()
    expected = [(lines[name], *fact) for name,facts in CASES.items() for fact in facts]
    actual = connection.execute('''select al.start_line,at.name,m.name,a.position,e.kind,
        case when e.kind=91 then it.name else v.str end
        from aggregate_field_init a
        left join exprs ag on ag.id=a.aggregate left join locations_expr al on al.id=ag.location
        left join expr_types am on am.id=ag.id left join usertypes at on at.id=am.typeid
        left join membervariables m on m.id=a.field
        left join exprs e on e.id=a.initializer
        left join expr_types em on em.id=e.id left join usertypes it on it.id=em.typeid
        left join valuebind vb on vb.expr=e.id left join "values" v on v.id=vb.val
        order by al.start_line,a.aggregate,a.position''').fetchall()
    if Counter(actual) != Counter(expected):
        errors.append('exact field/value/position facts: expected='+repr(sorted(expected))+' actual='+repr(actual))
    invalid = connection.execute('''select count(*) from aggregate_field_init a
        left join exprs ag on ag.id=a.aggregate left join exprs e on e.id=a.initializer
        left join membervariables m on m.id=a.field
        where ag.id is null or ag.kind<>91 or e.id is null or m.id is null
          or a.aggregate=a.initializer or a.position<0 or m.name='' ''').fetchone()[0]
    if invalid:
        errors.append('invalid aggregate/field/value identity: '+str(invalid))
    graph = connection.execute('''select a.aggregate,a.position from aggregate_field_init a
        left join exprparents p on p.parent_id=a.aggregate
            and p.expr_id=a.initializer and p.child_index=a.position
        where p.expr_id is null''').fetchall()
    if graph:
        errors.append('binding/main-tree disagreement: '+repr(graph))
    base_edges, identity_errors = expected_base_edges(connection, lines)
    errors.extend(identity_errors)
    for child, index, parent in base_edges:
        actual_edge = connection.execute('''select expr_id,child_index,parent_id
            from exprparents where parent_id=? and child_index=?''',
            (parent, index)).fetchall()
        if actual_edge != [(child, index, parent)]:
            errors.append('exact base parent edge: expected='+repr((child,index,parent))+
                          ' actual='+repr(actual_edge))
    self_edges = connection.execute('select * from exprparents where expr_id=parent_id').fetchall()
    if self_edges:
        errors.append('self parent edges: '+repr(self_edges))
    if connection.execute('pragma integrity_check').fetchall() != [('ok',)]:
        errors.append('database integrity')
    return errors


def mutations(database):
    before=hashlib.sha256(database.read_bytes()).hexdigest()
    changes={
      'wrong_field': 'update aggregate_field_init set field=(select min(id) from membervariables)',
      'wrong_value': 'update aggregate_field_init set initializer=(select min(id) from exprs where kind=123)',
      'wrong_position': 'update aggregate_field_init set position=position+20',
      'deleted_binding': 'delete from aggregate_field_init',
      'extra_binding': 'insert into aggregate_field_init select aggregate,initializer,field,1000 from aggregate_field_init limit 1',
      'self_initializer': 'update aggregate_field_init set initializer=aggregate',
      'wrong_literal': 'update "values" set str=\'CORRUPTED\'',
      'missing_parent': 'delete from exprparents where parent_id in (select aggregate from aggregate_field_init)',
      'missing_base_parent': delete_base_edges,
    }
    with sqlite3.connect(f'file:{database.resolve()}?mode=ro',uri=True) as original:
        with tempfile.TemporaryDirectory(prefix='arborchive-p8-negative-') as folder:
            for label,sql in changes.items():
                with sqlite3.connect(str(Path(folder)/(label+'.db'))) as copy:
                    original.backup(copy)
                    if callable(sql):
                        sql(copy)
                    else:
                        copy.executescript(sql)
                    if not validate(copy): raise AssertionError('mutation escaped: '+label)
    if hashlib.sha256(database.read_bytes()).hexdigest()!=before: raise AssertionError('source changed')
    print(f'P8_MUTATIONS_OK cases={len(changes)} source_unchanged=yes')


def main():
    parser=argparse.ArgumentParser();parser.add_argument('database',type=Path);parser.add_argument('--mutation-tests',action='store_true');args=parser.parse_args()
    if not args.database.is_file():parser.error('database does not exist')
    with sqlite3.connect(f'file:{args.database.resolve()}?mode=ro',uri=True) as c: errors=validate(c)
    if errors:
        for error in errors:print('P8_FAIL '+error)
        return 1
    print(f'P8_AGGREGATE_OK cases={len(CASES)} bindings={sum(map(len,CASES.values()))} base_edges={sum(map(len,BASE_EDGES.values()))} exact_field_value_position=yes')
    if args.mutation_tests:mutations(args.database)
    return 0

if __name__=='__main__':raise SystemExit(main())
