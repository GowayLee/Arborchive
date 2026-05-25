# Commit and PR Style

Arborchive patches should be small, reviewable, and easy to roll back.

## Commits

- Keep commits focused on one semantic subsystem or one documentation boundary.
- Prefer clear engineering intent over noisy file-by-file descriptions.
- Use the repository's existing commit style when practical.
- Do not include generated lockfiles or dependency artifacts unless the task
  explicitly requires them.
- Do not mix architecture cleanup with unrelated feature work.

Good commit messages explain the outcome:

```text
docs: add trellis-style agent context layer
```

## Pull Requests

PR descriptions should include:

- Summary.
- Motivation.
- Changes.
- Validation.
- Risks or known limitations.
- Follow-ups, if any.

For schema or semantic extraction PRs, include the CodeQL alignment statement,
affected tables, and representative validation queries.

For docs-only PRs, state that no behavior or schema files changed and list the
documentation validation performed.

## Review Boundaries

Each PR should make it obvious:

- which files were allowed to change;
- which files were intentionally not changed;
- which architecture boundary is being preserved;
- which command output proves the patch is ready for review.

If the patch cannot be reviewed independently, split it before opening the PR.
