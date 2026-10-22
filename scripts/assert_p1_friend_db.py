#!/usr/bin/env python3
"""Exact P1 friend owner/target contracts and corrupt-copy negative controls."""
import argparse
from collections import Counter
import hashlib
import re
from pathlib import Path
import sqlite3
import tempfile

FIXTURE = Path(__file__).resolve().parents[1] / 'tests/unit-tests/p1/friend_semantics_case.cc'
EXPECTATIONS = {
    'declared': ('Host', 'function', 'declared_friend', 'void declared_friend(Host &);'),
    'introduced': ('Host', 'function', 'introduced_friend', '// P1:introduced'),
    'repeated': ('Host', 'function', 'introduced_friend', '// P1:introduced'),
    'inline': ('Host', 'function', 'inline_friend', '// P1:inline'),
    'class': ('Host', 'class', 'Secret', None),
    'alias': ('Host', 'class', 'Secret', None),
    'introduced_class': ('Host', 'class', 'Introduced', None),
    'member': ('Host', 'function', 'access', 'struct MemberTarget {'),
    'overload_int': ('Host', 'function', 'overloaded', 'void overloaded(int);'),
    'overload_double': ('Host', 'function', 'overloaded', 'void overloaded(double);'),
    'other_owner': ('OtherHost', 'class', 'Secret', None),
    'shared_function': ('OtherHost', 'function', 'declared_friend', 'void declared_friend(Host &);'),
    'nested_owner': ('Inner', 'class', 'Secret', None),
    'concrete_in_template': ('DependentOwner', 'class', 'Secret', None),
    'system_alias_pod': ('TraitsHost', 'class', 'AliasPodTarget', None),
    'system_alias_nonpod': ('TraitsHost', 'class', 'AliasNonPodTarget', None),
    'system_alias_forward': ('TraitsHost', 'class', 'AliasForwardTarget', None),
    'system_direct_pod': ('TraitsHost', 'class', 'DirectPodTarget', None),
    'system_direct_nonpod': ('TraitsHost', 'class', 'DirectNonPodTarget', None),
    'system_direct_forward': ('TraitsHost', 'class', 'DirectForwardTarget', None),
}


# UserTypeKind and FuncType values from the existing database model.
OWNER_KINDS = {'Host': 2, 'OtherHost': 2, 'Inner': 1,
               'DependentOwner': 1, 'TraitsHost': 1}
CLASS_TARGET_KINDS = {'Secret': 2, 'Introduced': 1,
                      'AliasPodTarget': 2, 'AliasNonPodTarget': 2,
                      'AliasForwardTarget': 2, 'DirectPodTarget': 2,
                      'DirectNonPodTarget': 2, 'DirectForwardTarget': 2}
TRAIT_TABLES = ('is_complete', 'is_pod_class', 'is_standard_layout_class')
# Complete virtual classes are neither POD nor standard-layout; forward-only
# declarations must not acquire any of these positive facts.
TARGET_TRAITS = {
    'AliasPodTarget': (1, 1, 1),
    'AliasNonPodTarget': (1, 0, 0),
    'AliasForwardTarget': (0, 0, 0),
    'DirectPodTarget': (1, 1, 1),
    'DirectNonPodTarget': (1, 0, 0),
    'DirectForwardTarget': (0, 0, 0),
}


def source_range(line, line_number):
    # The fixture uses single-line declarations. Clang stores token-begin
    # columns, so class endpoints use the identifier start, not its last char.
    code = line.split('// P1:', 1)[0].rstrip().removesuffix(';').rstrip()
    tokens = list(re.finditer(r'[A-Za-z_]\w*|[^\s]', code))
    return (line_number, line.index('friend') + 1,
            line_number, tokens[-1].start() + 1)


def validate(connection):
    source = FIXTURE.read_text().splitlines()
    markers = {line.split('// P1:', 1)[1].strip(): i for i, line in enumerate(source, 1) if '// P1:' in line}
    errors = []
    if connection.execute('pragma integrity_check').fetchall() != [('ok',)]:
        errors.append('database integrity')
    columns = [r[1] for r in connection.execute('pragma table_info(frienddecls)')]
    if columns != ['id', 'type_id', 'decl_id', 'location']:
        errors.append('frienddecls columns')
    expected = []
    for marker, (owner, kind, target, declaration) in EXPECTATIONS.items():
        # Resolve existing declaration identities, never hardcode database IDs.
        owner_id = connection.execute('select min(id) from usertypes where name=?', (owner,)).fetchone()[0]
        if kind == 'class':
            target_id = connection.execute('select min(id) from usertypes where name=?', (target,)).fetchone()[0]
        else:
            if declaration.startswith('// P1:'):
                declaration_line = markers[declaration.split(':', 1)[1]]
            else:
                declaration_line = next(i for i, line in enumerate(source, 1) if declaration in line)
            target_id = connection.execute('''select min(f.id) from functions f
                join fun_decls d on d.function=f.id join locations_default l on l.id=d.location
                where f.name=? and l.start_line=?''', (target, declaration_line)).fetchone()[0]
        if owner_id is None or target_id is None:
            errors.append('missing fixture endpoint: ' + marker)
        if connection.execute('select kind from usertypes where id=?', (owner_id,)).fetchone() != (OWNER_KINDS[owner],):
            errors.append('wrong owner kind: ' + marker)
        target_table = 'usertypes' if kind == 'class' else 'functions'
        target_kind = CLASS_TARGET_KINDS[target] if kind == 'class' else 1
        if connection.execute(f'select kind from {target_table} where id=?', (target_id,)).fetchone() != (target_kind,):
            errors.append('wrong target kind: ' + marker)
        expected.append((*source_range(source[markers[marker] - 1], markers[marker]), owner_id, target_id))
    actual = connection.execute('''select l.start_line,l.start_column,l.end_line,l.end_column,f.type_id,f.decl_id from frienddecls f
        left join locations_default l on l.id=f.location order by l.start_line''').fetchall()
    if Counter(actual) != Counter(expected):
        errors.append('exact friend rows: expected=' + repr(sorted(expected)) + ' actual=' + repr(actual))
    for target, traits in TARGET_TRAITS.items():
        ids = connection.execute('select id from usertypes where name=?', (target,)).fetchall()
        if len(ids) != 1:
            errors.append('non-unique trait target: ' + target)
            continue
        target_id = ids[0][0]
        for table, expected_count in zip(TRAIT_TABLES, traits):
            actual_count = connection.execute(f'select count(*) from {table} where id=?', (target_id,)).fetchone()[0]
            if actual_count != expected_count:
                errors.append(f'target trait {target}.{table}: expected={expected_count} actual={actual_count}')
    invalid = connection.execute('''select count(*) from frienddecls f
        left join usertypes owner on owner.id=f.type_id
        left join usertypes target_type on target_type.id=f.decl_id
        left join functions target_function on target_function.id=f.decl_id
        left join locations_default l on l.id=f.location
        where f.type_id<0 or f.decl_id<0 or owner.id is null or l.id is null
        or (target_type.id is null and target_function.id is null)
        or l.start_line<1 or l.start_column<1 or l.end_line<l.start_line''').fetchone()[0]
    if invalid:
        errors.append('invalid owner/target/location references: ' + str(invalid))
    return errors


def mutations(database):
    before = hashlib.sha256(database.read_bytes()).hexdigest()
    changes = [
        ('deleted facts', 'delete from frienddecls'),
        ('missing owner', 'update frienddecls set type_id=-1'),
        ('wrong valid owner', "update frienddecls set type_id=(select min(id) from usertypes where name='OtherHost')"),
        ('swapped class ends', 'update frienddecls set type_id=decl_id,decl_id=type_id where decl_id in (select id from usertypes)'),
        ('wrong function target', "update frienddecls set decl_id=(select min(id) from functions where name='unrelated') where decl_id in (select id from functions)"),
        ('swapped overload', "update frienddecls set decl_id=(select min(id) from functions where name='overloaded') where decl_id in (select id from functions where name='overloaded')"),
        ('duplicate occurrence', 'insert into frienddecls select (select max(id)+1 from frienddecls),type_id,decl_id,location from frienddecls limit 1'),
        ('wrong source occurrence', 'update frienddecls set location=(select min(location) from frienddecls)'),
        ('dangling location', 'update frienddecls set location=-1'),
        ('wrong owner kind', "update usertypes set kind=5 where name='Host'"),
        ('wrong class target kind', "update usertypes set kind=5 where name='Secret'"),
        ('wrong function target kind', "update functions set kind=2 where name='declared_friend'"),
        ('wrong source columns', 'update locations_default set start_column=999,end_column=1000 where id in (select location from frienddecls)'),
        ('wrong source end line', 'update locations_default set end_line=end_line+1 where id in (select location from frienddecls)'),
    ]
    for table in TRAIT_TABLES:
        changes.extend([
            (f'missing {table}', f"delete from {table} where id=(select id from usertypes where name='AliasPodTarget')"),
            (f'misbound {table}', f"update {table} set id=(select id from usertypes where name='AliasForwardTarget') where id=(select id from usertypes where name='AliasPodTarget')"),
            (f'false forward {table}', f"insert into {table} select id from usertypes where name='AliasForwardTarget'"),
        ])
    for table in ('is_pod_class', 'is_standard_layout_class'):
        changes.append((f'false nonpod {table}', f"insert into {table} select id from usertypes where name='AliasNonPodTarget'"))
    with sqlite3.connect(f'file:{database.resolve()}?mode=ro', uri=True) as original:
        with tempfile.TemporaryDirectory(prefix='arborchive-p1-negative-') as folder:
            for index, (label, sql) in enumerate(changes):
                with sqlite3.connect(str(Path(folder) / f'{index}.db')) as altered:
                    original.backup(altered)
                    altered.executescript(sql)
                    if not validate(altered):
                        raise AssertionError('mutation escaped: ' + label)
    if hashlib.sha256(database.read_bytes()).hexdigest() != before:
        raise AssertionError('source database changed')
    print(f'P1_MUTATIONS_OK cases={len(changes)} source_unchanged=yes')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('database', type=Path)
    parser.add_argument('--mutation-tests', action='store_true')
    args = parser.parse_args()
    if not args.database.is_file():
        parser.error('database does not exist')
    with sqlite3.connect(f'file:{args.database.resolve()}?mode=ro', uri=True) as connection:
        errors = validate(connection)
    if errors:
        for error in errors:
            print('P1_FAIL ' + error)
        return 1
    print(f'P1_FRIEND_OK relations={len(EXPECTATIONS)} exact_owner_target=yes kinds=yes source_ranges=yes trait_targets={len(TARGET_TRAITS)}')
    if args.mutation_tests:
        mutations(args.database)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
