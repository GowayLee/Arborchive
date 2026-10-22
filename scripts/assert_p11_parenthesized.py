#!/usr/bin/env python3
"""Exact P11 conversion-source contracts and corruption controls for one fixture."""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import re
import sqlite3
import tempfile


SOURCE = Path(__file__).resolve().parents[1] / "tests/unit-tests/p11/parenthesized_sources_case.cc"
WRAPPERS = (8, 12, 210, 211, 212, 213, 214, 217)
CASES = {
    "scalar_plain": (84, 0, "numeric"),
    "scalar_paren": (84, 1, "numeric"),
    "scalar_nested": (84, 2, "numeric"),
    "member_plain": (74, 0, "numeric"),
    "member_paren": (74, 1, "numeric"),
    "member_nested": (74, 2, "numeric"),
    "arrow_plain": (74, 0, "numeric"),
    "arrow_paren": (74, 2, "numeric"),
    "routine_plain": (97, 0, "decay"),
    "routine_paren": (97, 1, "decay"),
    "routine_nested": (97, 2, "decay"),
    "address_plain": (2, 0, "address"),
    "address_outer": (2, 1, "address"),
    "address_inner": (97, 2, "address"),
    "method_dot_plain": (74, 0, "method"),
    "method_dot_paren": (74, 0, "method"),
    "method_dot_nested": (74, 0, "method"),
    "method_arrow_plain": (74, 0, "method"),
    "method_arrow_paren": (74, 0, "method"),
    "method_arrow_nested": (74, 0, "method"),
    "callback_dot_plain": (74, 0, "callback"),
    "callback_dot_nested": (74, 2, "callback"),
    "callback_arrow_nested": (74, 2, "callback"),
}


def source_lines() -> dict[str, int]:
    result = {}
    for line, text in enumerate(SOURCE.read_text().splitlines(), 1):
        match = re.search(r"// CHECK: (\w+)$", text)
        if match:
            if match[1] in result:
                raise ValueError("duplicate source marker: " + match[1])
            result[match[1]] = line
    if result.keys() != CASES.keys():
        raise ValueError("fixture/check marker mismatch")
    return result


def verify(connection: sqlite3.Connection) -> list[str]:
    errors = []

    def check(label, actual, expected):
        if actual != expected:
            errors.append(f"{label}: expected={expected!r}, actual={actual!r}")

    def rows(sql, params=()):
        return connection.execute(sql, params).fetchall()

    def check_parameter(name, identity, parameter_name):
        check(name + " parameter identity", rows("""
            select f.name,p."index",d.name from varbind b
            join params p on p.id=b.var
            join functions f on f.id=p.function
            join var_decls d on d.variable=b.var
            where b.expr=?
            """, (identity,)), [(name, 0, parameter_name)])

    check("integrity", rows("pragma integrity_check"), [("ok",)])
    check("finished", rows("select count(*) from compilation_finished"), [(1,)])
    check("one type per expression", rows("""
        select e.id from exprs e left join expr_types t on t.id=e.id
        group by e.id having count(t.id)<>1
        """), [])
    check("metadata references", rows("""
        select t.id from expr_types t left join exprs e on e.id=t.id
        where e.id is null or t.typeid<0 or t.value_category not in (1,2,3)
        """), [])
    check("conversion endpoints", rows("""
        select c.conversion from exprconv c
        left join exprs a on a.id=c.converted left join exprs b on b.id=c.conversion
        where a.id is null or b.id is null
        """), [])
    check("wrapper incoming edge", rows("""
        select e.id from exprs e left join exprconv c on c.conversion=e.id
        where e.kind in (8,12,210,211,212,213,214)
        group by e.id having count(c.converted)<>1
        """), [])
    check("main tree excludes wrappers", rows("""
        select p.expr_id from exprparents p join exprs a on a.id=p.expr_id
        left join exprs b on b.id=p.parent_id
        where a.kind in (8,12,210,211,212,213,214,217)
           or b.kind in (8,12,210,211,212,213,214,217)
        """), [])
    check("load excludes wrappers", rows("""
        select l.expr_id from expr_isload l left join exprs e on e.id=l.expr_id
        where e.id is null or e.kind in (8,12,210,211,212,213,214,217)
        """), [])
    check("one main parent per expression", rows("""
        select expr_id from exprparents group by expr_id having count(*)>1
        """), [])

    for name, line in source_lines().items():
        source_kind, parentheses, category = CASES[name]
        expressions = rows("""
            select e.id,e.kind,l.start_column
            from exprs e join locations_expr l on l.id=e.location
            where l.start_line=? order by l.start_column desc
            """, (line,))
        by_kind = {}
        for identity, kind, column in expressions:
            by_kind.setdefault(kind, []).append(identity)
        is_arrow = name.startswith("arrow_") or "_arrow_" in name
        if category == "method":
            # A bound method is not a field read or a conversion source.
            # Retain only the real call and its receiver, even with parentheses.
            check(name + " exact nodes",
                  {kind: len(ids) for kind, ids in by_kind.items()}, {74: 1, 84: 1})
            calls = rows("""
                select e.id from exprs e join iscall i on i.caller=e.id
                join locations_expr l on l.id=e.location
                where l.start_line=? and i.kind=1
                """, (line,))
            check(name + " real call count", len(calls), 1)
            if len(calls) != 1 or len(by_kind.get(84, [])) != 1:
                continue
            call, receiver = calls[0][0], by_kind[84][0]
            check(name + " method binding", rows("""
                select f.name from funbind b join functions f on f.id=b.fun
                where b.expr=?
                """, (call,)), [("method",)])
            check_parameter(name, receiver, "box")
            check(name + " receiver exact parent", rows("""
                select child_index,parent_id from exprparents where expr_id=?
                """, (receiver,)), [(-1, call)])
            check(name + " call type/category", rows("""
                select b.name,t.value_category from expr_types t
                join builtintypes b on b.id=t.typeid where t.id=?
                """, (call,)), [("int", 1)])
            check(name + " exact loads", rows("""
                select x.expr_id from expr_isload x join exprs e on e.id=x.expr_id
                join locations_expr l on l.id=e.location where l.start_line=?
                """, (line,)), [(receiver,)] if is_arrow else [])
            continue
        kinds = [source_kind] + [12] * parentheses
        if category in ("numeric", "decay"):
            kinds += [214]
        if category == "numeric":
            kinds += [210]
        if source_kind == 74:
            kinds += [84]  # The field's receiver is part of the same source line.
        if name == "address_inner":
            kinds += [2]
        expected_counts = {kind: kinds.count(kind) for kind in set(kinds)}
        actual_counts = {kind: len(ids) for kind, ids in by_kind.items()}
        check(name + " nodes", actual_counts, expected_counts)
        if actual_counts != expected_counts:
            continue
        # Parentheses sort inner -> outer by their source columns, not runtime ID.
        nodes = [by_kind[source_kind][0]] + by_kind.get(12, [])
        nodes += by_kind.get(214, []) + by_kind.get(210, [])
        expected_edges = set(zip(nodes, nodes[1:]))
        actual_edges = rows("""
            select c.converted,c.conversion from exprconv c
            join exprs e on e.id=c.conversion join locations_expr l on l.id=e.location
            where l.start_line=?
            """, (line,))
        check(name + " exact chain", set(actual_edges), expected_edges)
        check(name + " unique chain", len(actual_edges), len(expected_edges))

        if source_kind == 84:
            check_parameter(name, nodes[0], "value")
        if source_kind == 74:
            receiver = by_kind[84][0]
            check_parameter(name, receiver, "box")
            check(name + " member read kind", rows(
                "select kind from iscall where caller=?", (nodes[0],)),
                [(6 if is_arrow else 5,)])
            check(name + " member exact base", rows("""
                select expr_id,child_index from exprparents where parent_id=?
                """, (nodes[0],)), [(receiver, -1)])
            check(name + " base exact parent", rows("""
                select child_index,parent_id from exprparents where expr_id=?
                """, (receiver,)), [(-1, nodes[0])])

        source_type = rows("select typeid,value_category from expr_types where id=?", (nodes[0],))
        source_category = 1 if source_kind == 2 else 3
        if len(source_type) == 1:
            check(name + " source category", source_type[0][1], source_category)
            if source_kind == 97:
                # Existing TypeProcessor allocates a fresh routine type ID on
                # each visit. Compare this fixture's exact int(int) signature
                # and qualifiers, not unrelated numeric IDs. Other expression
                # families retain strict type-ID equality below.
                for routine_node in [nodes[0]] + by_kind.get(12, []):
                    check(name + " routine return/category", rows("""
                        select b.name,t.value_category from expr_types t
                        join routinetypes r on r.id=t.typeid
                        join builtintypes b on b.id=r.return_type where t.id=?
                        """, (routine_node,)), [("int", 3)])
                    check(name + " routine exact arguments", rows("""
                        select a."index",b.name from expr_types t
                        join routinetypeargs a on a.routine=t.typeid
                        left join builtintypes b on b.id=a.type_id where t.id=?
                        order by a."index"
                        """, (routine_node,)), [(0, "int")])
                    check(name + " routine qualifiers", rows("""
                        select s.spec_id from expr_types t
                        join typespecifiers s on s.type_id=t.typeid where t.id=?
                        """, (routine_node,)), [])
            else:
                for paren in by_kind.get(12, []):
                    check(name + " paren type/category",
                          rows("select typeid,value_category from expr_types where id=?", (paren,)),
                          source_type)
        if category == "numeric":
            check(name + " source type", rows("""
                select b.name from expr_types t join builtintypes b on b.id=t.typeid
                where t.id=?
                """, (nodes[0],)), [("int",)])
            for cast in by_kind[214] + by_kind[210]:
                check(name + " cast type/category", rows("""
                    select b.name,t.value_category from expr_types t
                    join builtintypes b on b.id=t.typeid where t.id=?
                    """, (cast,)), [("double", 1)])
        elif category == "decay":
            check(name + " decayed pointer/category", rows("""
                select d.kind,d.name,t.value_category from expr_types t
                join derivedtypes d on d.id=t.typeid where t.id=?
                """, (by_kind[214][0],)), [(6, "int (*)(int)", 1)])
        elif category == "callback":
            check(name + " callback pointer/category", rows("""
                select d.kind,d.name,t.value_category from expr_types t
                join derivedtypes d on d.id=t.typeid where t.id=?
                """, (nodes[0],)), [(6, "int (*)(int)", 3)])
        if source_kind == 97:
            check(name + " routine identity", rows("""
                select f.name from funbind b join functions f on f.id=b.fun
                where b.expr=?
                """, (nodes[0],)), [("target",)])
        expected_loads = {nodes[0]} if category in ("numeric", "callback") else set()
        if is_arrow:
            check(name + " pointer base", len(by_kind.get(84, [])), 1)
            expected_loads.update(by_kind.get(84, []))
        actual_loads = rows("""
            select load.expr_id from expr_isload load join exprs e on e.id=load.expr_id
            join locations_expr l on l.id=e.location where l.start_line=?
            """, (line,))
        check(name + " exact loads", {r[0] for r in actual_loads}, expected_loads)
        expected_generated = set(by_kind.get(214, []))
        check(name + " generated wrappers", {r[0] for r in rows("""
            select g.id from compgenerated g join exprs e on e.id=g.id
            join locations_expr l on l.id=e.location where l.start_line=?
            """, (line,))}, expected_generated)
        check(name + " cast kinds", set(rows("""
            select k.expr_id,k.kind from conversionkinds k join exprs e on e.id=k.expr_id
            join locations_expr l on l.id=e.location where l.start_line=?
            """, (line,))), {(identity, 0) for identity in by_kind.get(214, []) + by_kind.get(210, [])})
        if category == "address":
            check(name + " address expression", len(by_kind.get(2, [])), 1)
    return errors


def mutations(database: Path) -> int:
    """A corrupted copy must fail the same semantic checker; source stays read-only."""
    before = hashlib.sha256(database.read_bytes()).hexdigest()
    line = source_lines()["member_nested"]
    predicate = f"select e.id from exprs e join locations_expr l on l.id=e.location where l.start_line={line}"
    routine_line = source_lines()["routine_nested"]
    routine_types = f"""select t.typeid from expr_types t join exprs e on e.id=t.id
        join locations_expr l on l.id=e.location
        where l.start_line={routine_line} and e.kind in (97,12)"""

    def case_exprs(name, kind):
        return f"""select e.id from exprs e join locations_expr l on l.id=e.location
            where l.start_line={source_lines()[name]} and e.kind={kind}"""

    scalar = case_exprs("scalar_nested", 84)
    other_scalar = case_exprs("scalar_plain", 84)
    member_base = case_exprs("member_nested", 84)
    other_member_base = case_exprs("member_plain", 84)
    member = case_exprs("member_nested", 74)
    method = case_exprs("method_dot_nested", 74)
    method_base = case_exprs("method_dot_nested", 84)
    other_method = case_exprs("method_dot_plain", 74)
    callback = case_exprs("callback_dot_nested", 74)
    callback_parens = case_exprs("callback_dot_nested", 12)
    corruptions = {
        "missing_chain": f"delete from exprconv where conversion in ({predicate})",
        "reversed_chain": f"update exprconv set converted=conversion,conversion=converted where conversion in ({predicate})",
        "wrong_chain_source": f"update exprconv set converted=(select min(id) from exprs where kind=97) where conversion in ({predicate})",
        "load_on_wrapper": f"update expr_isload set expr_id=(select max(e.id) from exprs e where e.id in ({predicate}) and e.kind=12) where expr_id in ({predicate})",
        "wrong_category": f"update expr_types set value_category=1 where id in ({predicate})",
        "wrong_type": f"update expr_types set typeid=(select min(id) from builtintypes where name='double') where id in ({predicate})",
        "wrong_routine_return": f"update routinetypes set return_type=(select min(id) from builtintypes where name='double') where id in ({routine_types})",
        "wrong_routine_argument": f"update routinetypeargs set type_id=(select min(id) from builtintypes where name='double') where routine in ({routine_types})",
        "wrong_decay_kind": "update derivedtypes set kind=1 where kind=6",
        "missing_routine_binding": "delete from funbind where expr in (select id from exprs where kind=97)",
        "wrong_generated_wrapper": "insert into compgenerated select min(id) from exprs where kind=12",
        "wrapper_in_main_tree": "insert into exprparents select min(id),0,max(id) from exprs where kind=12",
        "duplicate_type_row": "insert into expr_types select * from expr_types where id=(select min(id) from exprs)",
        "missing_scalar_binding": f"delete from varbind where expr in ({scalar})",
        "wrong_scalar_binding": f"update varbind set var=(select var from varbind where expr in ({other_scalar})) where expr in ({scalar})",
        "missing_member_binding": f"delete from varbind where expr in ({member_base})",
        "wrong_member_binding": f"update varbind set var=(select var from varbind where expr in ({other_member_base})) where expr in ({member_base})",
        "missing_member_read_kind": f"delete from iscall where caller in ({member})",
        "wrong_member_read_kind": f"update iscall set kind=1 where caller in ({member})",
        "missing_member_base": f"delete from exprparents where parent_id in ({member})",
        "extra_member_parent": f"insert into exprparents select id,-1,({other_method}) from exprs where id in ({member_base})",
        "method_as_field_read": f"update iscall set kind=5 where caller in ({method})",
        "wrong_method_binding": f"update funbind set fun=(select id from functions where name='target') where expr in ({method})",
        "missing_method_receiver_parent": f"delete from exprparents where expr_id in ({method_base})",
        "extra_method_receiver_parent": f"insert into exprparents select id,-1,({other_method}) from exprs where id in ({method_base})",
        "missing_callback_chain": f"delete from exprconv where conversion in ({callback_parens})",
        "wrong_callback_read_kind": f"update iscall set kind=6 where caller in ({callback})",
        "missing_callback_load": f"delete from expr_isload where expr_id in ({callback})",
    }
    with tempfile.TemporaryDirectory(prefix="arborchive-p11-mutations-") as temporary:
        with sqlite3.connect(f"file:{database.resolve()}?mode=ro", uri=True) as source:
            for name, sql in corruptions.items():
                with sqlite3.connect(Path(temporary) / (name + ".db")) as copy:
                    source.backup(copy)
                    previous_changes = copy.total_changes
                    copy.executescript(sql)
                    if copy.total_changes == previous_changes:
                        raise AssertionError("mutation changed no rows: " + name)
                    if not verify(copy):
                        raise AssertionError("mutation accepted: " + name)
                    print("[p11_mutation] " + name + ": REJECTED")
    if hashlib.sha256(database.read_bytes()).hexdigest() != before:
        raise AssertionError("source database changed during mutation controls")
    return len(corruptions)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("database", type=Path)
    parser.add_argument("--mutations", action="store_true")
    args = parser.parse_args()
    if not args.database.is_file():
        parser.error("database does not exist")
    with sqlite3.connect(f"file:{args.database.resolve()}?mode=ro", uri=True) as connection:
        errors = verify(connection)
    if errors:
        for error in errors:
            print("[p11_parenthesized] ERROR " + error)
        print(f"[p11_parenthesized] FAIL ({len(errors)} errors)")
        return 1
    print(f"[p11_parenthesized] PASS ({len(CASES)} source cases)")
    if args.mutations:
        count = mutations(args.database)
        print(f"[p11_mutation] PASS ({count} corruptions rejected; source unchanged)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
