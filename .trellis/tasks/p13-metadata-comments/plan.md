# P13 Metadata & Comment Safe Subset Plan

## Goal

Complete CodeQL-aligned compilation metadata and the function documentation
comment safe subset while explicitly deferring XML and unrelated tail tables.

## Scope

- `compilation_compiling_files`, manual build mode, shared compiler arguments,
  compilation-finished identity, and extractor version.
- `comments` and `commentbinding` for main-file explicit functions only.
- P13 fixture, SQLite assertions, roadmap, datatable status, XML audit.

## Non-goals

XML parsing, new CLI inputs, fileannotations, ordinary/macro/non-function
comments, P7, CFG, VLA, name qualifiers, links, packages, and diagnostics.

## Architecture

`ClangASTManager` owns the single compiler-argument construction API.
`CompRecorder` owns invocation metadata. `CommentProcessor` owns comment
filtering, formatting, location, canonical binding, and deduplication;
`FunctionProcessor` injects that collaboration without new visitor orchestration.

## Validation

Baseline and modified `scripts/test_all.sh`, ORM generation, LLVM 19 debug build,
diff check, P13 DB summary/queries, invalid-source finished guard, and rollback
verification on a separate copy.
