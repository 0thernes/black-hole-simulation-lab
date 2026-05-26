# Project Log

## 2026-05-26: Repo Recovery Baseline

Problem:

- The local workspace contained real black-hole simulation seed files that were
  not tracked in Git.
- GitHub only had the initial repository plus workflow commits.
- Previous package-based work was not durable enough for ongoing A/B tests,
  E2E tests, audits, and reviews.

Actions:

- Confirmed `origin` points to GitHub.
- Added and pushed direct local package-intake workflow.
- Hardened package selection so unrelated ZIP files are not selected.
- Reorganized source philosophy/audit inputs under `docs/source/`.
- Added CMake, CTest, CI, validation, architecture, research, testing, workflow,
  gameplan, changelog, and project log.
- Prepared the current C++ seed, data schema, Python harness, and research docs
  for a real baseline commit.

Known caveats:

- `Nautilus/` is a nested Orca/Git worktree and is intentionally excluded from
  the project baseline.
- The current C++ program is an analytic seed, not a full simulator.
- Existing audit docs are seed frameworks and need evidence mapping as modules
  mature.
