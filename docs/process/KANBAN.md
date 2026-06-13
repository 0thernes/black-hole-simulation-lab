# Kanban Board

A lightweight, file-based Kanban board. It is the single place to see what
is in flight, what is queued, and what is done. Cards move left → right.
Keep it honest: a card is only in **Done** when it is merged, tested, and
the relevant gate passes.

## Columns and policies

| Column | Entry policy | Exit policy |
|---|---|---|
| **Backlog** | Any idea, gap, or audit finding. No commitment. | Pulled into Ready when scoped with an acceptance check. |
| **Ready** | Scoped: has a clear acceptance check and an owner. | Pulled into In Progress when capacity is free. |
| **In Progress** | Actively being built. **WIP limit: 2.** | Code complete + local tests/validation pass. |
| **Review** | Awaiting build/test/CI confirmation or self-review. | CI green and acceptance check met. |
| **Done** | Merged to `main`, pushed, CI green. | Archived after a milestone closes. |

WIP limit on In Progress is 2 to force finishing over starting. This is a
solo/AI-paced project; the limit keeps commits focused.

Card id format: `K-NNN`. Cards that came from the audit reference the
finding id (e.g. `F-005`).

---

## Backlog

- `K-101` Geodesic right-hand side: Schwarzschild null-geodesic derivative
  functor feeding `rk45_integrate` (N=8). [milestone 1]
- `K-102` Add `model_status` column to `runs` + exporter + harness. [audit
  F-013/F-018 cluster, data-model item 1]
- `K-103` Adopt strong `units` types in the metrics layer or document the
  raw-double carve-out per function. [audit F-005]
- `K-104` JSON exporter (`data/json_writer.hpp`) mirroring the CSV writer.
- `K-105` SQLite exporter in C++ converging with `data/schema.sql` and the
  Python harness.
- `K-106` E2E test: run executable → emit data → validate schema ingest.
- `K-107` A/B integrator harness (RK4 vs DP45: error, runtime, steps).
- `K-108` PI step-size controller for `rk45_integrate` (fewer rejections).
- `K-109` Kerr null geodesics (Carter constant; equatorial first).
- `K-110` CPU photon-ring image (one ray per pixel; lensed disk).
- `K-111` CUDA port of the ray marcher (RTX 5070 Ti). [headline visual goal]
- `K-112` First `tsotchke/libirrep` integration per ADR-0005.
- `K-113` Expand brain corpus toward the full 700-profile roster.
- `K-114` `.gitattributes` explicit eol rules for `.ps1`/`.sh`/binary.
  [audit F-024 cluster]
- `K-115` Remaining audit majors not yet ticketed (triage from the
  2026-06-12 report; 47 open findings).
- `K-116` Add a `model_status` truth-tier column to the data model so exact
  analytic, numerical, and placeholder values are row-level labeled.
  [inspection S20.09/10, S12 cluster; milestone M2]
- `K-117` Wire a real Lyapunov estimator (variational/shadow-trajectory)
  to replace the now-NULL placeholder. [inspection S20.12 follow-up; M1+]
- `K-118` Implement producers for `trajectories` and `chaos_stats` (or mark
  them reserved). [inspection S12.09; M2]
- `K-119` Separate `theory_version` from `git_commit` in the harness and
  fix the dim_ensemble seed-row collision. [inspection S21.04/05]
- `K-120` Add a Python linter/type-checker (ruff + mypy) with a CI gate
  mirroring clang-format. [inspection S16.12]
- `K-121` Pin GitHub Actions to commit SHAs; add gitleaks secret scanning
  to CI. [inspection S08.09, S18.11]
- `K-122` Adopt strong `units` types in the metrics API (or document the
  raw-double carve-out per function). [inspection S04.06; same as K-103]

## Ready

- `K-101` Geodesic RHS — acceptance: a null ray at large impact parameter
  deflects by the weak-field GR value 4GM/(c^2 b) within tolerance; a ray
  at the critical impact parameter asymptotes to the photon sphere.
- `K-114` `.gitattributes` eol rules — acceptance: fresh clone on Windows
  produces an LF pre-commit hook and no CRLF churn warnings on `.ps1`.

## In Progress

*(empty — WIP capacity available)*

## Review

*(empty)*

## Done

- `K-001` Repo bootstrap iterations 1-8 (foundation, skeleton, vision,
  brain corpus, source cards, integration plan, modularization, daily
  workflow).
- `K-002` Full 67-finding adversarial audit (`F-001`..`F-067`).
- `K-003` Remediation fixes 1-5: CI green, shadow factor-of-2, license
  (AGPL-3.0), generator determinism + drift gate, gates-that-gate.
- `K-004` Numerical integrators (RK4 + adaptive Dormand-Prince 5(4)) with
  convergence tests and complexity reference.
- `K-005` Comprehensive documentation suite (INDEX, GLOSSARY, ERM/data
  dictionary, ROADMAP, KANBAN, COMPLEXITY, Engineering Resource Plan).
- `K-006` 500-point / 25-section grounded inspection
  (`docs/audits/INSPECTION-500-POINT.md`).
- `K-007` CI/CD hardening: 3-job pipeline, clang-format enforcement
  (pinned 19.1.7), .gitattributes eol policy, Dependabot.
- `K-008` Inspection remediation batches 1-3: ~25 FAILs closed (repo
  hygiene, schema honesty, CLI/CSV tests, AGPL licensing, referential
  integrity gates, deterministic LF generators).

---

## How to update this board

1. Move the card line to its new column.
2. If it entered **Done**, add the commit/PR reference inline.
3. When a milestone closes, archive its Done cards under a dated heading
   at the bottom (keep the board scannable).
4. Respect the WIP limit. If In Progress is full, finish something before
   starting.

The board is reviewed at the start of each working session and reconciled
against `docs/planning/ROADMAP.md` and the open audit findings.
