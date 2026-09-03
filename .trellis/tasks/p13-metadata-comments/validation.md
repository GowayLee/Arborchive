# P13 Validation Contract

- Four new tables have exact CodeQL column shapes.
- One compiling-file row uses `num=0` and resolves to compilation/file rows.
- Manual build mode is 1; finished reuses compilation ID; failure has no finished row.
- Recorded arguments are the same complete ordered sequence used by ClangTool.
- ClangTool's default syntax-only adjuster is cleared so it cannot append an
  unrecorded `-fsyntax-only` argument.
- Version row is `arborchive/1.0.0` plus the LLVM 19 frontend version.
- Only main-file explicit function documentation comments are stored.
- Text is formatted without delimiters; location is the raw comment range.
- Comment locations use the main `files.id` as their CodeQL `@container`, and
  the P13 assertion rejects orphaned location containers.
- Binding resolves to a function entity and canonical redeclarations deduplicate.
- Temporary location results are released through RAII after row assembly.
- XML/fileannotations remain absent.
