# 250-Point Audit Baseline

This file is the root audit ledger for OPPENGROK Black Hole Simulation Lab.
Detailed historical seed audit material lives in
`docs/audits/INITIAL_250_POINT_GOLD_STANDARD_AUDIT.md`.

The audit is intentionally staged. This project is not yet a finished science
lab; the current goal is a clean, reviewable foundation that can support
repeated A/B tests, E2E tests, research reviews, and release audits.

## Current Baseline

Date: 2026-05-26

Repository state goals:

- GitHub remote configured.
- No ZIP/package workflow as final source of truth.
- Source, research, architecture, test strategy, and project log committed.
- Build/test scaffold present.
- Generated files ignored.
- Nested local worktrees excluded.
- Research claims separated by truth label.

## Section 1: Coding

1. C++ targets C++20 or newer.
2. Physical quantities use strong types where implemented.
3. Analytic formulas need tests before expansion.
4. Public interfaces must be small and documented.
5. Hot-path numerical code must avoid hidden allocation.
6. Constants must be named and source-backed.
7. Generated or experimental code must be labeled.
8. Compiler warnings should trend toward zero.
9. Formatting should become automated before large refactors.
10. No secrets or local-only state in source files.

## Section 2: Development

11. `README.md` explains status, workflow, and limits.
12. `CHANGELOG.md` records notable changes.
13. `docs/reports/PROJECT_LOG.md` records operational history.
14. `docs/process/REPO_WORKFLOW.md` defines daily workflow.
15. `scripts/Validate-ResearchOS.py` validates baseline structure.
16. CMake and CTest are the default compiled test path.
17. CI runs validation, configure, build, and tests.
18. Package intake uses `scripts/local/Sync-OppengrokLocalRepo.ps1`.
19. `_incoming/`, `build/`, and `exports/` are ignored.
20. Nested worktree `Nautilus/` is not part of project source.

## Section 3: Engineering

21. Schwarzschild and Kerr analytic seed calculations are the first truth anchor.
22. Future numerical approximations require documented tolerance.
23. Future observational constraints require source cards.
24. Python reference harness and C++ exporter must converge on schema semantics.
25. SQLite schema is the starting data contract.
26. A/B testing is required for competing numerical kernels.
27. E2E tests are required for CLI-to-data workflows.
28. Reproducibility requires seed, git commit, parameters, and truth label.
29. Heavy research checks should be separated from quick local checks.
30. Performance claims require measurements.

## Section 4: Architecture

31. Architecture is documented in `docs/architecture/ARCHITECTURE.md`.
32. Physics code, data export, visualization, and research docs stay separate.
33. Visualization consumes validated state; it does not own physics.
34. Source cards live under `docs/research/`.
35. Test strategy lives under `docs/testing/`.
36. Planning lives under `docs/planning/`.
37. Operations live under `docs/operations/`.
38. Large source/audit inputs live under `docs/source/`.
39. New modules should be introduced as libraries before new executables.
40. Every architecture shift needs a project-log entry.

## Section 5: Design

41. Default UX should favor researchers reviewing data, not marketing.
42. Visuals must identify what is measured and what is illustrative.
43. Accessibility requirements apply to future visual output.
44. CLI output should become machine-readable where possible.
45. Exported artifacts need provenance.
46. A/B reports need winner, loser, metrics, and rollback.
47. Documentation should be sober about what is implemented.
48. Speculative physics must not be presented as validated simulation output.
49. Every release needs validation evidence.
50. This audit grows from 50 baseline items to a full evidence-mapped 250 as
    modules mature.

## Expansion Plan

The next audit expansion should add 40 items per section:

- 1-50 Coding
- 51-100 Development
- 101-150 Engineering
- 151-200 Architecture
- 201-250 Design

Each item must have one of these states:

- `PASS`: evidence exists in code, tests, docs, or CI.
- `PARTIAL`: started, but missing evidence.
- `PLANNED`: not implemented yet.
- `BLOCKED`: waiting on a named prerequisite.

The audit should not claim a full PASS until validation evidence exists.
