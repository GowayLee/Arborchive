#!/usr/bin/env python3
"""Executable SQLite assertions for Arborchive regression fixtures."""

from __future__ import annotations

import argparse
import sqlite3
import sys
from pathlib import Path


class Audit:
    def __init__(self, case: str, database: Path) -> None:
        self.case = case
        self.database = database
        self.connection = sqlite3.connect(database)
        self.errors = 0
        self.warnings = 0

    def close(self) -> None:
        self.connection.close()

    def scalar(self, sql: str):
        row = self.connection.execute(sql).fetchone()
        return None if row is None else row[0]

    def check(self, label: str, sql: str, expected, *, warning: bool = False) -> None:
        try:
            actual = self.scalar(sql)
        except sqlite3.Error as error:
            self._report(label, f"SQL error: {error}", warning)
            return
        if actual != expected:
            self._report(label, f"expected={expected!r}, actual={actual!r}", warning)

    def check_columns(
        self, table: str, expected: list[str], *, warning: bool = False
    ) -> None:
        try:
            actual = [
                row[1]
                for row in self.connection.execute(f'pragma table_info("{table}")')
            ]
        except sqlite3.Error as error:
            self._report(f"{table} columns", f"SQL error: {error}", warning)
            return
        if actual != expected:
            self._report(
                f"{table} columns",
                f"expected={expected!r}, actual={actual!r}",
                warning,
            )

    def warn_if_positive(self, label: str, sql: str) -> None:
        try:
            actual = self.scalar(sql)
        except sqlite3.Error as error:
            self._report(label, f"SQL error: {error}", True)
            return
        if actual and actual > 0:
            self._report(label, f"actual={actual}", True)

    def _report(self, label: str, detail: str, warning: bool) -> None:
        if warning:
            self.warnings += 1
            level = "WARN"
        else:
            self.errors += 1
            level = "ERROR"
        print(f"[{level}] {self.case}: {label}: {detail}", file=sys.stderr)

    def finish(self) -> int:
        if self.errors:
            print(
                f"[assert_db] {self.case}: FAIL "
                f"({self.errors} errors, {self.warnings} warnings)",
                file=sys.stderr,
            )
            return 1
        print(f"[assert_db] {self.case}: PASS ({self.warnings} warnings)")
        return 0


def check_count(audit: Audit, table: str, expected: int) -> None:
    audit.check(f"{table} count", f'select count(*) from "{table}"', expected)


def check_common(audit: Audit) -> None:
    audit.check("database integrity", "pragma integrity_check", "ok")
    audit.check("compilation row", "select count(*) from compilations", 1)
    audit.check("finished compilation", "select count(*) from compilation_finished", 1)
    audit.check("source file rows", "select count(*) >= 1 from files", 1)
    audit.check_columns("compilation_compiling_files", ["id", "num", "file"])
    audit.check_columns(
        "extractor_version", ["codeql_version", "frontend_version"]
    )
    audit.check(
        "manual compilation mode", "select mode from compilation_build_mode", 1
    )
    audit.check(
        "single build mode row", "select count(*) from compilation_build_mode", 1
    )
    audit.check(
        "single extractor version row", "select count(*) from extractor_version", 1
    )
    audit.check(
        "dense unique compilation argument indexes",
        """
        select count(*)=count(distinct num)
           and min(num)=0 and max(num)=count(*)-1
        from compilation_args
        """,
        1,
    )
    audit.check(
        "compiling file relation",
        """
        select count(*) from compilation_compiling_files relation
        join compilations compilation on compilation.id=relation.id
        join files file on file.id=relation.file
        where relation.num=0 and relation.id >= 0 and relation.file >= 0
        """,
        1,
    )
    audit.check(
        "finished compilation identity",
        """
        select count(*) from compilation_finished finished
        join compilations compilation on compilation.id=finished.id
        where finished.id >= 0
        """,
        1,
    )
    audit.check(
        "extractor schema version",
        "select codeql_version from extractor_version",
        "arborchive/1.0.0",
    )
    audit.check(
        "LLVM 19 frontend version",
        "select frontend_version glob '19.*' from extractor_version",
        1,
    )
    audit.check(
        "compiler executable first",
        """
        select arg glob '*/clang*' from compilation_args
        where num=(select min(num) from compilation_args)
        """,
        1,
    )
    audit.check(
        "resource directory second",
        "select arg like '-resource-dir=%' from compilation_args where num=1",
        1,
    )
    audit.check(
        "source argument last",
        """
        select arg like '%.cc' from compilation_args
        where num=(select max(num) from compilation_args)
        """,
        1,
    )


def check_p13(audit: Audit) -> None:
    audit.check_columns("comments", ["id", "contents", "location"])
    audit.check_columns("commentbinding", ["id", "element"])
    check_count(audit, "comments", 3)
    check_count(audit, "commentbinding", 3)
    audit.check(
        "configured compiler argument order",
        """
        select group_concat(arg, '|') from (
          select arg from compilation_args where num between 2 and 6 order by num
        )
        """,
        "-Itests/unit-tests/p13/include|-DP13_CONFIG_DEFINE=13|-std=c++20|-fexceptions|-O0",
    )
    audit.check(
        "formatted documentation contents",
        """
        select count(*) from comments
        where contents in (
          'Returns its argument unchanged.',
          'Adds one to the supplied value.\n\n The block markers are removed from persisted contents.',
          'Returns the stable member value.'
        )
        """,
        3,
    )
    audit.check(
        "comment delimiter removal",
        """
        select count(*) from comments
        where contents like '%///%' or contents like '%/**%' or contents like '%//!%'
        """,
        0,
    )
    audit.check(
        "comment references",
        """
        select count(*) from comments comment
        left join locations_default location on location.id=comment.location
        left join commentbinding binding on binding.id=comment.id
        left join functions function on function.id=binding.element
        where comment.id < 0 or comment.location < 0 or binding.element < 0
           or location.id is null or binding.id is null or function.id is null
        """,
        0,
    )
    audit.check(
        "comment location containers resolve to files",
        """
        select count(*) from comments comment
        join locations_default location on location.id=comment.location
        left join files file on file.id=location.container
        where location.container < 0 or file.id is null
        """,
        0,
    )
    audit.check(
        "canonical redeclaration dedup",
        """
        select count(*) from commentbinding binding
        join functions function on function.id=binding.element
        where function.name='p13_documented'
          and binding.element=(
            select min(id) from functions where name='p13_documented'
          )
        """,
        1,
    )
    audit.check(
        "macro function comment deferred",
        """
        select count(*) from commentbinding binding
        join functions function on function.id=binding.element
        where function.name='p13_macro_documented'
        """,
        0,
    )
    audit.check(
        "raw comment source ranges",
        """
        select count(*) from comments comment
        join locations_default location on location.id=comment.location
        where (location.start_line=3 and location.end_line=3)
           or (location.start_line=8 and location.end_line=12)
           or (location.start_line=26 and location.end_line=26)
        """,
        3,
    )
    for table in (
        "xmlEncoding", "xmlDTDs", "xmlElements", "xmlAttrs", "xmlNs",
        "xmlHasNs", "xmlComments", "xmlChars", "xmllocations",
        "fileannotations",
    ):
        audit.check(
            f"{table} remains deferred",
            f"select count(*) from pragma_table_list where name='{table}'",
            0,
        )


def check_namespace(audit: Audit) -> None:
    expected_counts = {
        "namespaces": 14,
        "namespace_decls": 18,
        "namespace_inline": 1,
        "usings": 10,
        "using_container": 8,
    }
    for table, expected in expected_counts.items():
        check_count(audit, table, expected)

    audit.check(
        "reopened namespace identity",
        """
        select count(*) from (
          select n.name
          from namespaces n
          join namespace_decls d on d.namespace_id=n.id
          where n.name in ('multi_block','p4_decl_reopened','p4_reopened','using_reopened')
          group by n.name
          having count(distinct n.id)=1 and count(d.id)=2
        )
        """,
        4,
    )
    audit.check(
        "using kind distribution",
        """
        select count(*) from (
          select kind, count(*) as n from usings group by kind
          having (kind=1 and n=7) or (kind=2 and n=3)
        )
        """,
        2,
    )
    audit.check(
        "using owner orphans",
        """
        select count(*)
        from using_container c
        left join usings u on u.id=c.child
        left join namespaces n on n.id=c.parent
        left join files f on f.id=c.parent
        where u.id is null or (n.id is null and f.id is null)
        """,
        0,
    )
    audit.warn_if_positive(
        "known P4 namespacembrs unresolved references",
        "select count(*) from namespacembrs where memberid < 0",
    )
    audit.check_columns("namespacembrs", ["parentid", "memberid"])
    audit.check(
        "CodeQL memberid unique constraint",
        """
        select count(*) from pragma_index_list('namespacembrs') il
        where il."unique"=1
          and (select count(*) from pragma_index_info(il.name))=1
          and exists (
            select 1 from pragma_index_info(il.name) ii where ii.name='memberid'
          )
        """,
        1,
        warning=True,
    )


def check_p5(audit: Audit) -> None:
    counts = {
        "unit-tests/p5/hierarchy_case": (6, 7, 5, 1, 8),
        "unit-tests/p5/layout_case": (3, 4, 2, 1, 9),
        "unit-tests/p5/semantic_gaps_case": (9, 11, 5, 3, 21),
    }
    derivations, specifiers, direct, virtual, fields = counts[audit.case]
    for table, expected in (
        ("derivations", derivations),
        ("derspecifiers", specifiers),
        ("direct_base_offsets", direct),
        ("virtual_base_offsets", virtual),
        ("fieldoffsets", fields),
    ):
        check_count(audit, table, expected)

    audit.check(
        "derivation reference orphans",
        """
        select count(*) from derivations d
        left join usertypes sub on sub.id=d.sub
        left join usertypes super on super.id=d.super
        left join locations_default l on l.id=d.location
        where sub.id is null or super.id is null or l.id is null
        """,
        0,
    )
    audit.check(
        "derivation specifier orphans",
        """
        select count(*) from derspecifiers ds
        left join derivations d on d.derivation=ds.der_id
        left join specifiers s on s.id=ds.spec_id
        where d.derivation is null or s.id is null
        """,
        0,
    )
    audit.check(
        "direct offset orphans",
        """
        select count(*) from direct_base_offsets o
        left join derivations d on d.derivation=o.der_id
        where d.derivation is null
        """,
        0,
    )
    audit.check(
        "virtual offset orphans",
        """
        select count(*) from virtual_base_offsets o
        left join usertypes sub on sub.id=o.sub
        left join usertypes super on super.id=o.super
        where sub.id is null or super.id is null
        """,
        0,
    )
    audit.check(
        "field offset orphans",
        """
        select count(*) from fieldoffsets f
        left join membervariables m on m.id=f.id
        where m.id is null
        """,
        0,
    )
    audit.check_columns(
        "derivations",
        ["derivation", "sub", "index", "super", "location"],
        warning=True,
    )
    audit.warn_if_positive(
        "known P5 membervariable type placeholders",
        """
        select count(*) from fieldoffsets f
        join membervariables m on m.id=f.id
        where m.type_id < 0
        """,
    )


def check_p6(audit: Audit) -> None:
    check_count(audit, "lambdas", 14)
    check_count(audit, "lambda_capture", 14)
    audit.check(
        "lambda reference orphans",
        """
        select count(*) from lambdas l
        left join exprs e on e.id=l.expr
        where e.id is null
        """,
        0,
    )
    audit.check(
        "lambda capture reference orphans",
        """
        select count(*) from lambda_capture c
        left join lambdas l on l.expr=c.lambda
        left join membervariables m on m.id=c.field
        left join locations_default loc on loc.id=c.location
        where l.expr is null or m.id is null or loc.id is null
        """,
        0,
    )
    audit.check(
        "duplicate lambda capture indexes",
        """
        select count(*) from (
          select lambda, "index", count(*) n from lambda_capture
          group by lambda, "index" having n > 1
        )
        """,
        0,
    )
    audit.warn_if_positive(
        "known P6 capture field type placeholders",
        """
        select count(*) from lambda_capture c
        join membervariables m on m.id=c.field
        where m.type_id < 0
        """,
    )


P7_COUNTS: dict[str, dict[str, int]] = {
    "unit-tests/p7/attribute_presence_case": {
        "attributes": 1,
        "funcattributes": 1,
    },
    "unit-tests/p7/attribute_arguments_case": {
        "attributes": 2,
        "funcattributes": 2,
        "attribute_args": 2,
        "attribute_arg_value": 1,
        "attribute_arg_constant": 1,
    },
    "unit-tests/p7/attribute_owner_links_case": {
        "attributes": 9,
        "typeattributes": 3,
        "varattributes": 4,
        "stmtattributes": 2,
    },
    "unit-tests/p7/attribute_string_args_case": {
        "attributes": 3,
        "attribute_args": 4,
        "attribute_arg_value": 4,
    },
    "unit-tests/p7/attribute_constant_args_case": {
        "attributes": 3,
        "attribute_args": 3,
        "attribute_arg_constant": 3,
    },
    "unit-tests/p7/attribute_expr_args_case": {
        "attributes": 4,
        "attribute_args": 3,
        "attribute_arg_constant": 1,
        "attribute_arg_expr": 2,
    },
    "unit-tests/p7/attribute_assume_aligned_expr_args_case": {
        "attributes": 3,
        "attribute_args": 2,
        "attribute_arg_expr": 2,
    },
    "unit-tests/p7/attribute_extra_value_args_case": {
        "attributes": 2,
        "attribute_args": 1,
        "attribute_arg_value": 1,
    },
}


def check_p7(audit: Audit) -> None:
    for table, expected in P7_COUNTS[audit.case].items():
        check_count(audit, table, expected)
    check_count(audit, "attribute_arg_type", 0)
    check_count(audit, "attribute_arg_name", 0)
    audit.check(
        "attribute location orphans",
        """
        select count(*) from attributes a
        left join locations_default l on l.id=a.location
        where l.id is null or a.id < 0 or a.location < 0
        """,
        0,
    )
    audit.check(
        "function attribute owner orphans",
        """
        select count(*) from funcattributes o
        left join functions f on f.id=o.func_id
        left join attributes a on a.id=o.spec_id
        where f.id is null or a.id is null
        """,
        0,
    )
    audit.check(
        "type attribute owner orphans",
        """
        with type_ids(id) as (
          select id from types union select id from builtintypes
          union select id from derivedtypes union select id from usertypes
          union select id from routinetypes
        )
        select count(*) from typeattributes o
        left join type_ids t on t.id=o.type_id
        left join attributes a on a.id=o.spec_id
        where t.id is null or a.id is null
        """,
        0,
    )
    audit.check(
        "variable attribute owner orphans",
        """
        with variable_ids(id) as (
          select id from globalvariables union select id from localvariables
          union select id from membervariables union select id from params
        )
        select count(*) from varattributes o
        left join variable_ids v on v.id=o.var_id
        left join attributes a on a.id=o.spec_id
        where v.id is null or a.id is null
        """,
        0,
    )
    audit.check(
        "statement attribute owner orphans",
        """
        select count(*) from stmtattributes o
        left join stmts s on s.id=o.stmt_id
        left join attributes a on a.id=o.spec_id
        where s.id is null or a.id is null
        """,
        0,
    )
    audit.check(
        "attribute argument owner orphans",
        """
        select count(*) from attribute_args aa
        left join attributes a on a.id=aa.attribute
        where a.id is null or aa.id < 0 or aa.attribute < 0 or aa.location < 0
        """,
        0,
    )
    for table, column in (
        ("attribute_arg_value", "arg"),
        ("attribute_arg_constant", "arg"),
        ("attribute_arg_expr", "arg"),
        ("attribute_arg_type", "arg"),
        ("attribute_arg_name", "arg"),
    ):
        audit.check(
            f"{table} argument orphans",
            f"""
            select count(*) from {table} p
            left join attribute_args aa on aa.id=p.{column}
            where aa.id is null or p.{column} < 0
            """,
            0,
        )
    audit.check(
        "constant payload expression orphans",
        """
        select count(*) from attribute_arg_constant p
        left join exprs e on e.id=p.constant
        where e.id is null
        """,
        0,
    )
    audit.check(
        "expression payload orphans",
        """
        select count(*) from attribute_arg_expr p
        left join exprs e on e.id=p.expr
        where e.id is null
        """,
        0,
    )
    audit.check(
        "attribute payload overlap",
        """
        select count(*) from (
          select arg from (
            select arg from attribute_arg_value
            union all select arg from attribute_arg_constant
            union all select arg from attribute_arg_expr
            union all select arg from attribute_arg_type
            union all select arg from attribute_arg_name
          ) payloads
          group by arg having count(*) > 1
        ) overlaps
        """,
        0,
    )


def check_p8(audit: Audit) -> None:
    for table, expected in (
        ("initialisers", 8),
        ("braced_initialisers", 6),
        ("aggregate_field_init", 4),
        ("aggregate_array_init", 5),
    ):
        check_count(audit, table, expected)
    audit.check(
        "initialiser reference orphans",
        """
        select count(*) from initialisers i
        left join var_decls v on v.variable=i.var
        left join exprs e on e.id=i.expr
        left join locations_expr l on l.id=i.location
        where v.variable is null or e.id is null or l.id is null
           or i.var < 0 or i.expr < 0 or i.location < 0
        """,
        0,
    )
    audit.check(
        "braced initialiser orphans",
        """
        select count(*) from braced_initialisers b
        left join initialisers i on i.init=b.init
        where i.init is null
        """,
        0,
    )
    audit.check(
        "aggregate field reference orphans",
        """
        select count(*) from aggregate_field_init a
        left join exprs aggregate_expr on aggregate_expr.id=a.aggregate
        left join exprs initializer_expr on initializer_expr.id=a.initializer
        left join membervariables m on m.id=a.field
        where aggregate_expr.id is null or initializer_expr.id is null or m.id is null
        """,
        0,
    )
    audit.check(
        "CodeQL initialiser expr unique constraint",
        """
        select count(*) from pragma_index_list('initialisers') il
        join pragma_index_info(il.name) ii
        where il."unique"=1 and ii.name='expr'
        """,
        1,
        warning=True,
    )
    audit.check(
        "aggregate array reference orphans",
        """
        select count(*) from aggregate_array_init a
        left join exprs aggregate_expr on aggregate_expr.id=a.aggregate
        left join exprs initializer_expr on initializer_expr.id=a.initializer
        where aggregate_expr.id is null or initializer_expr.id is null
           or a.element_index <> a.position
        """,
        0,
    )


def check_p9(audit: Audit) -> None:
    expected_columns = {
        "if_initialization": ["if_stmt", "init_id"],
        "constexpr_if_initialization": ["constexpr_if_stmt", "init_id"],
        "constexpr_if_then": ["constexpr_if_stmt", "then_id"],
        "constexpr_if_else": ["constexpr_if_stmt", "else_id"],
        "consteval_if_then": ["constexpr_if_stmt", "then_id"],
        "consteval_if_else": ["constexpr_if_stmt", "else_id"],
    }
    for table, columns in expected_columns.items():
        audit.check_columns(table, columns)
        audit.check(
            f"{table} owner primary key",
            f"select count(*) from pragma_table_info('{table}') "
            f"where name='{columns[0]}' and pk=1",
            1,
        )

    audit.check(
        "correct if initialization table exists",
        "select count(*) from sqlite_master "
        "where type='table' and name='if_initialization'",
        1,
    )
    audit.check(
        "misspelled if initialization table removed",
        "select count(*) from sqlite_master "
        "where type='table' and name='if_initalization'",
        0,
    )

    expected_kinds = {1: 2, 2: 2, 35: 3, 38: 1, 39: 1}
    for kind, expected in expected_kinds.items():
        audit.check(
            f"statement kind {kind} count",
            f"select count(*) from stmts where kind={kind}",
            expected,
        )

    for table, expected in (
        ("if_initialization", 2),
        ("if_then", 2),
        ("if_else", 2),
        ("constexpr_if_initialization", 2),
        ("constexpr_if_then", 3),
        ("constexpr_if_else", 3),
        ("consteval_if_then", 2),
        ("consteval_if_else", 2),
    ):
        check_count(audit, table, expected)

    relation_checks = (
        (
            "constexpr_if_initialization",
            "init_id",
            "child.kind not in (1,17)",
            "owner.kind<>35",
        ),
        ("constexpr_if_then", "then_id", "child.kind<>7", "owner.kind<>35"),
        ("constexpr_if_else", "else_id", "child.kind<>7", "owner.kind<>35"),
        (
            "consteval_if_then",
            "then_id",
            "child.kind<>7",
            "owner.kind not in (38,39)",
        ),
        (
            "consteval_if_else",
            "else_id",
            "child.kind<>7",
            "owner.kind not in (38,39)",
        ),
    )
    for table, child_column, child_mismatch, owner_mismatch in relation_checks:
        audit.check(
            f"{table} reference orphans",
            f"""
            select count(*) from {table} relation
            left join stmts owner on owner.id=relation.constexpr_if_stmt
            left join stmts child on child.id=relation.{child_column}
            where owner.id is null or {owner_mismatch}
               or child.id is null or {child_mismatch}
               or relation.{child_column} < 0
            """,
            0,
        )

    audit.check(
        "ordinary if initialization references",
        """
        select count(*) from if_initialization relation
        left join stmts owner on owner.id=relation.if_stmt
        left join stmts child on child.id=relation.init_id
        where owner.id is null or owner.kind<>2 or child.id is null
           or child.kind not in (1,17)
           or relation.init_id < 0
        """,
        0,
    )
    for table, owner_column, child_column in (
        ("if_initialization", "if_stmt", "init_id"),
        (
            "constexpr_if_initialization",
            "constexpr_if_stmt",
            "init_id",
        ),
    ):
        audit.check(
            f"{table} declaration/expression initializer distribution",
            f"""
            select count(*) from (
              select child.kind, count(*) as n
              from {table} relation
              join stmts owner on owner.id=relation.{owner_column}
              join stmts child on child.id=relation.{child_column}
              group by child.kind
              having (child.kind=1 and n=1) or (child.kind=17 and n=1)
            )
            """,
            2,
        )
        audit.check(
            f"{table} expression initializer reuses expr id",
            f"""
            select count(*) from {table} relation
            join stmts child on child.id=relation.{child_column}
            left join exprs expression on expression.id=child.id
            where child.kind=1 and expression.id is null
            """,
            0,
        )
    for table in ("consteval_if_then", "consteval_if_else"):
        audit.check(
            f"{table} owner kind distribution",
            f"""
            select count(*) from (
              select owner.kind, count(*) as n
              from {table} relation
              join stmts owner on owner.id=relation.constexpr_if_stmt
              group by owner.kind
              having (owner.kind=38 and n=1) or (owner.kind=39 and n=1)
            )
            """,
            2,
        )


def check_p10(audit: Audit) -> None:
    check_count(audit, "exprparents", 69)
    audit.check_columns("exprparents", ["expr_id", "child_index", "parent_id"])
    audit.check(
        "exprparents child orphans",
        """
        select count(*) from exprparents relation
        left join exprs child on child.id=relation.expr_id
        where child.id is null or relation.expr_id < 0
        """,
        0,
    )
    audit.check(
        "exprparents parent orphans",
        """
        select count(*) from exprparents relation
        left join exprs expression on expression.id=relation.parent_id
        left join stmts statement on statement.id=relation.parent_id
        left join initialisers initialiser on initialiser.init=relation.parent_id
        where relation.parent_id < 0
           or (expression.id is null and statement.id is null
               and initialiser.init is null)
        """,
        0,
    )
    audit.check(
        "exprparents duplicate relations",
        """
        select count(*) from (
          select expr_id, child_index, parent_id
          from exprparents
          group by expr_id, child_index, parent_id
          having count(*) > 1
        )
        """,
        0,
    )
    audit.check(
        "exprparents child has one main parent",
        """
        select count(*) from (
          select expr_id from exprparents group by expr_id having count(*) > 1
        )
        """,
        0,
    )
    audit.check(
        "expression-parent self loops",
        """
        select count(*) from exprparents relation
        join exprs parent on parent.id=relation.parent_id
        left join stmts expression_statement
          on expression_statement.id=relation.parent_id
         and expression_statement.kind=1
        where relation.expr_id=relation.parent_id
          and expression_statement.id is null
        """,
        0,
    )
    audit.check(
        "conversion nodes excluded from main graph",
        """
        select count(*) from exprparents relation
        join exprs child on child.id=relation.expr_id
        left join exprs parent on parent.id=relation.parent_id
        where child.kind in (3,5,8,12,210,211,212,213,214,217)
           or parent.kind in (3,5,8,12,210,211,212,213,214,217)
        """,
        0,
    )
    audit.check(
        "conditional child indices",
        """
        select count(*) from (
          select relation.parent_id
          from exprparents relation
          join exprs parent on parent.id=relation.parent_id
          where parent.kind=24
          group by relation.parent_id
          having count(*)=3 and count(distinct relation.child_index)=3
             and min(relation.child_index)=0 and max(relation.child_index)=2
        )
        """,
        1,
    )
    audit.check(
        "unary child index",
        """
        select count(*) from exprparents relation
        join exprs parent on parent.id=relation.parent_id
        where parent.kind in (22,23) and relation.child_index=0
          and relation.expr_id<>relation.parent_id
        """,
        3,
    )
    audit.check(
        "binary child indices",
        """
        select count(*) from (
          select relation.parent_id
          from exprparents relation
          join exprs parent on parent.id=relation.parent_id
          where parent.kind in (25,27,46,47,52,53)
            and relation.expr_id<>relation.parent_id
          group by relation.parent_id
          having min(relation.child_index)=0 and max(relation.child_index)=1
             and count(*)=2
        )
        """,
        13,
    )
    audit.check(
        "array child indices",
        """
        select count(*) from (
          select relation.parent_id
          from exprparents relation
          join exprs parent on parent.id=relation.parent_id
          where parent.kind=68
          group by relation.parent_id
          having min(relation.child_index)=0 and max(relation.child_index)=1
             and count(*)=2
        )
        """,
        3,
    )
    audit.check(
        "init-list child indices",
        """
        select count(*) from exprparents relation
        join exprs parent on parent.id=relation.parent_id
        where parent.kind=91 and relation.child_index in (0,1)
        """,
        2,
    )
    audit.check(
        "sizeof expression child index",
        """
        select count(*) from exprparents relation
        join exprs parent on parent.id=relation.parent_id
        where parent.kind=93 and relation.child_index=0
        """,
        1,
    )
    audit.check(
        "direct call argument index",
        """
        select count(*) from (
          select relation.parent_id
          from exprparents relation
          join exprs parent on parent.id=relation.parent_id
          where parent.kind=74 and relation.expr_id<>relation.parent_id
          group by relation.parent_id
          having min(relation.child_index)=0 and max(relation.child_index)=0
             and count(*)=1
        )
        """,
        3,
    )
    audit.check(
        "indirect call child indices",
        """
        select count(*) from (
          select relation.parent_id
          from exprparents relation
          join exprs parent on parent.id=relation.parent_id
          where parent.kind=74 and relation.expr_id<>relation.parent_id
          group by relation.parent_id
          having min(relation.child_index)=0 and max(relation.child_index)=1
             and count(*)=2
        )
        """,
        1,
    )
    audit.check(
        "member call qualifier index",
        """
        select count(*) from (
          select relation.parent_id
          from exprparents relation
          join exprs parent on parent.id=relation.parent_id
          where parent.kind=74 and relation.expr_id<>relation.parent_id
          group by relation.parent_id
          having min(relation.child_index)=-1 and max(relation.child_index)=0
             and count(*)=2
        )
        """,
        1,
    )
    for label, statement_kind, child_index in (
        ("if condition index", 2, 1),
        ("while condition index", 3, 0),
        ("do condition index", 8, 0),
        ("switch condition index", 11, 1),
        ("return expression index", 6, 0),
    ):
        audit.check(
            label,
            f"""
            select count(*) from exprparents relation
            join stmts parent on parent.id=relation.parent_id
            where parent.kind={statement_kind}
              and relation.child_index={child_index}
            """,
            1 if statement_kind not in (6,) else 3,
        )
    audit.check(
        "for condition and update indices",
        """
        select count(*) from exprparents relation
        join stmts parent on parent.id=relation.parent_id
        where parent.kind=9 and relation.child_index in (1,2)
        """,
        2,
    )
    audit.check(
        "expression statement roots",
        """
        select count(*) from exprparents relation
        join stmts parent on parent.id=relation.parent_id
        where parent.kind=1 and relation.child_index=0
          and relation.expr_id=relation.parent_id
        """,
        7,
    )
    audit.check(
        "parenthesized expression statement normalized",
        """
        select count(*) from exprparents relation
        join stmts parent on parent.id=relation.parent_id
        join exprs child on child.id=relation.expr_id
        where parent.kind=1 and child.kind=84
          and relation.child_index=0
          and relation.expr_id=relation.parent_id
        """,
        1,
    )
    audit.check(
        "initialiser roots",
        """
        select count(*) from exprparents relation
        join initialisers parent
          on parent.init=relation.parent_id and parent.expr=relation.expr_id
        where relation.child_index=0
        """,
        5,
    )


def check_p11(audit: Audit) -> None:
    expected_counts = {
        "exprs": 56,
        "exprconv": 30,
        "expr_types": 56,
        "expr_isload": 18,
        "compgenerated": 14,
        "conversionkinds": 26,
    }
    for table, expected in expected_counts.items():
        check_count(audit, table, expected)

    for table, columns in (
        ("exprconv", ["converted", "conversion"]),
        ("expr_types", ["id", "typeid", "value_category"]),
        ("expr_isload", ["expr_id"]),
        ("compgenerated", ["id"]),
        ("conversionkinds", ["expr_id", "kind"]),
    ):
        audit.check_columns(table, columns)

    audit.check(
        "every expression has one type/category row",
        """
        select count(*) from (
          select expression.id
          from exprs expression
          left join expr_types typed on typed.id=expression.id
          group by expression.id having count(typed.id)<>1
        )
        """,
        0,
    )
    audit.check(
        "all value categories covered",
        """
        select count(*) from (
          select value_category from expr_types
          group by value_category having value_category in (1,2,3)
        )
        """,
        3,
    )
    audit.check(
        "invalid value categories",
        "select count(*) from expr_types where value_category not in (1,2,3)",
        0,
    )
    audit.check(
        "conversion endpoint orphans",
        """
        select count(*) from exprconv relation
        left join exprs converted on converted.id=relation.converted
        left join exprs conversion on conversion.id=relation.conversion
        where converted.id is null or conversion.id is null
        """,
        0,
    )
    audit.check(
        "every wrapper has one incoming conversion edge",
        """
        select count(*) from (
          select expression.id
          from exprs expression
          left join exprconv relation on relation.conversion=expression.id
          where expression.kind in (8,12,210,211,212,213,214)
          group by expression.id having count(relation.converted)<>1
        )
        """,
        0,
    )
    audit.check(
        "nested conversion chain",
        """
        select count(*) > 0 from exprconv outer_edge
        join exprconv inner_edge on inner_edge.conversion=outer_edge.converted
        """,
        1,
    )
    audit.check(
        "paren load conversion chain order",
        """
        select count(*) from exprconv paren_edge
        join exprs source
          on source.id=paren_edge.converted and source.kind=84
        join exprs paren
          on paren.id=paren_edge.conversion and paren.kind=12
        join exprconv implicit_edge on implicit_edge.converted=paren.id
        join exprs implicit_cast
          on implicit_cast.id=implicit_edge.conversion
         and implicit_cast.kind=214
        join exprconv explicit_edge
          on explicit_edge.converted=implicit_cast.id
        join exprs explicit_cast
          on explicit_cast.id=explicit_edge.conversion
         and explicit_cast.kind=210
        """,
        1,
    )
    audit.check(
        "function-to-pointer decay source",
        """
        select count(*) from exprconv relation
        join exprs source on source.id=relation.converted and source.kind=97
        join exprs conversion
          on conversion.id=relation.conversion and conversion.kind=214
        join compgenerated generated on generated.id=conversion.id
        """,
        1,
    )
    audit.check(
        "nullptr implicit conversion source",
        """
        select count(*) from exprconv relation
        join exprs source on source.id=relation.converted and source.kind=123
        join exprs conversion
          on conversion.id=relation.conversion and conversion.kind=214
        join conversionkinds kind
          on kind.expr_id=conversion.id and kind.kind=0
        join compgenerated generated on generated.id=conversion.id
        """,
        1,
    )
    audit.check(
        "unchecked derived-to-base conversion kind",
        """
        select count(*) from exprconv relation
        join exprs source on source.id=relation.converted and source.kind=85
        join conversionkinds kind
          on kind.expr_id=relation.conversion and kind.kind=2
        join compgenerated generated on generated.id=relation.conversion
        """,
        1,
    )
    audit.check(
        "load markers resolve to non-wrapper expressions",
        """
        select count(*) from expr_isload load
        left join exprs expression on expression.id=load.expr_id
        where expression.id is null
           or expression.kind in (8,12,210,211,212,213,214,217)
        """,
        0,
    )
    audit.check(
        "implicit generated kind distribution",
        """
        select count(*) from (
          select expression.kind, count(*) n
          from compgenerated generated
          join exprs expression on expression.id=generated.id
          group by expression.kind
          having (expression.kind=8 and n=1)
              or (expression.kind=214 and n=13)
        )
        """,
        2,
    )
    audit.check(
        "explicit cast syntax distribution",
        """
        select count(*) from (
          select expression.kind, count(*) n
          from exprs expression
          left join compgenerated generated on generated.id=expression.id
          where expression.kind between 210 and 214 and generated.id is null
          group by expression.kind
          having (expression.kind=210 and n=6)
              or (expression.kind=211 and n=2)
              or (expression.kind=212 and n=1)
              or (expression.kind=213 and n=1)
              or (expression.kind=214 and n=3)
        )
        """,
        5,
    )
    audit.check(
        "every cast has exactly one conversion kind",
        """
        select count(*) from (
          select expression.id
          from exprs expression
          left join conversionkinds kind on kind.expr_id=expression.id
          where expression.kind between 210 and 214
          group by expression.id having count(kind.kind)<>1
        )
        """,
        0,
    )
    audit.check(
        "conversion kind category coverage",
        """
        select count(*) from (
          select kind from conversionkinds
          group by kind having kind in (0,1,2,3,4,5,6)
        )
        """,
        7,
    )
    audit.check(
        "paren and array conversion-kind exclusion",
        """
        select count(*) from conversionkinds kind
        join exprs expression on expression.id=kind.expr_id
        where expression.kind in (8,12)
        """,
        0,
    )
    audit.check("legacy implicit cast kind absent", "select count(*) from exprs where kind=217", 0)
    audit.check(
        "conversion nodes excluded from exprparents",
        """
        select count(*) from exprparents relation
        join exprs child on child.id=relation.expr_id
        left join exprs parent on parent.id=relation.parent_id
        where child.kind in (8,12,210,211,212,213,214,217)
           or parent.kind in (8,12,210,211,212,213,214,217)
        """,
        0,
    )


def check_p12(audit: Audit) -> None:
    expected_counts = {
        "expr_allocator": 8,
        "expr_deallocator": 17,
        "new_allocated_type": 5,
        "new_array_allocated_type": 3,
    }
    for table, expected in expected_counts.items():
        check_count(audit, table, expected)

    for table, columns in (
        ("expr_allocator", ["expr", "func", "form"]),
        ("expr_deallocator", ["expr", "func", "form"]),
        ("new_allocated_type", ["expr", "type_id"]),
        ("new_array_allocated_type", ["expr", "type_id"]),
    ):
        audit.check_columns(table, columns)

    audit.check(
        "new/delete kind distribution",
        """
        select count(*) from (
          select kind, count(*) n from exprs
          where kind in (87,88,128,129) group by kind
          having (kind=87 and n=5)
              or (kind=88 and n=8)
              or (kind=128 and n=2)
              or (kind=129 and n=3)
        )
        """,
        4,
    )
    audit.check(
        "allocator form distribution",
        """
        select count(*) from (
          select form, count(*) n from expr_allocator group by form
          having (form=0 and n=7) or (form=1 and n=1)
        )
        """,
        2,
    )
    audit.check(
        "deallocator form coverage",
        """
        select count(*) from (
          select form from expr_deallocator group by form
          having form in (0,1,2,3,4)
        )
        """,
        5,
    )
    audit.check(
        "allocator/deallocator function reference orphans",
        """
        select count(*) from (
          select func from expr_allocator
          union all
          select func from expr_deallocator
        ) relation
        left join functions function on function.id=relation.func
        where relation.func < 0 or function.id is null
        """,
        0,
    )
    audit.check(
        "allocation expression reference orphans",
        """
        select count(*) from (
          select expr from expr_allocator
          union all select expr from expr_deallocator
          union all select expr from new_allocated_type
          union all select expr from new_array_allocated_type
        ) relation
        left join exprs expression on expression.id=relation.expr
        where expression.id is null
        """,
        0,
    )
    audit.check(
        "allocated type reference orphans",
        """
        with all_types(id) as (
          select id from builtintypes
          union select id from derivedtypes
          union select id from usertypes
          union select id from routinetypes
        ), allocated(type_id) as (
          select type_id from new_allocated_type
          union all select type_id from new_array_allocated_type
        )
        select count(*) from allocated
        left join all_types on all_types.id=allocated.type_id
        where allocated.type_id < 0 or all_types.id is null
        """,
        0,
    )
    audit.check(
        "array allocated outer type shapes",
        """
        select count(*) from new_array_allocated_type allocated
        join derivedtypes type on type.id=allocated.type_id and type.kind=4
        where type.name in ('int[3]','int[]','int[][4]')
        """,
        3,
    )
    audit.check(
        "nested fixed array base resolved",
        """
        select count(*) from new_array_allocated_type allocated
        join derivedtypes outer_type on outer_type.id=allocated.type_id
        join derivedtypes inner_type on inner_type.id=outer_type.type_id
        where outer_type.name='int[][4]' and inner_type.name='int[4]'
        """,
        1,
    )
    audit.check(
        "array allocated element references resolved",
        """
        select count(*) from new_array_allocated_type allocated
        join derivedtypes outer_type on outer_type.id=allocated.type_id
        where outer_type.type_id < 0
        """,
        0,
    )
    audit.check(
        "initializer child one edges",
        """
        select count(*) from exprparents relation
        join exprs parent on parent.id=relation.parent_id
        where parent.kind in (87,129) and relation.child_index=1
        """,
        2,
    )
    audit.check(
        "dynamic extent child two edges",
        """
        select count(*) from exprparents relation
        join exprs parent on parent.id=relation.parent_id
        where parent.kind=129 and relation.child_index=2
        """,
        2,
    )
    audit.check(
        "trivial delete child three edges",
        """
        select count(*) from exprparents relation
        join exprs parent on parent.id=relation.parent_id
        where parent.kind in (88,128) and relation.child_index=3
        """,
        5,
    )
    audit.check(
        "nontrivial delete child three guard",
        """
        select count(*) from exprparents relation
        join exprs parent on parent.id=relation.parent_id
        join locations_expr location on location.id=parent.location
        where location.start_line in (76,77,82,83,84)
          and relation.child_index=3
        """,
        0,
    )
    audit.check(
        "virtual destructor static deallocator guard",
        """
        select count(*) from expr_deallocator relation
        join exprs expression on expression.id=relation.expr
        join locations_expr location on location.id=expression.location
        where location.start_line=77
        """,
        0,
    )
    audit.check(
        "statically selected virtual-type deallocators",
        """
        select count(*) from expr_deallocator relation
        join exprs expression on expression.id=relation.expr
        join locations_expr location on location.id=expression.location
        where location.start_line in (82,83,84)
        """,
        3,
    )
    audit.check(
        "dependent allocation expressions deferred",
        """
        select count(*) from exprs expression
        join locations_expr location on location.id=expression.location
        where expression.kind in (87,88,128,129)
          and location.start_line in (49,54)
        """,
        0,
    )
    audit.check(
        "deferred lifetime tables absent",
        """
        select count(*) from pragma_table_list
        where name in ('synthetic_destructor_call','expr_reuse')
        """,
        0,
    )


def run(case: str, database: Path) -> int:
    audit = Audit(case, database)
    try:
        check_common(audit)
        if case == "unit-tests/namespace":
            check_namespace(audit)
        elif case.startswith("unit-tests/p5/"):
            check_p5(audit)
        elif case == "unit-tests/p6/lambda_case":
            check_p6(audit)
        elif case.startswith("unit-tests/p7/"):
            check_p7(audit)
        elif case == "unit-tests/p8/initialization_case":
            check_p8(audit)
        elif case == "unit-tests/p9/constexpr_flow_case":
            check_p9(audit)
        elif case == "unit-tests/p10/expression_graph_case":
            check_p10(audit)
        elif case == "unit-tests/p11/casts_conversion_case":
            check_p11(audit)
        elif case == "unit-tests/p12/allocation_lifetime_case":
            check_p12(audit)
        elif case == "unit-tests/p13/metadata_comments_case":
            check_p13(audit)
        else:
            audit.warn_if_positive(
                "known non-P8 initialiser expression placeholders",
                "select count(*) from initialisers where expr < 0",
            )
        return audit.finish()
    finally:
        audit.close()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("case")
    parser.add_argument("database", type=Path)
    args = parser.parse_args()
    if not args.database.is_file():
        parser.error(f"database does not exist: {args.database}")
    return run(args.case, args.database)


if __name__ == "__main__":
    raise SystemExit(main())
