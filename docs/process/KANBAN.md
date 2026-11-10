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
- `K-125` Star field on the Kerr disk, once `trace_kerr_pixel` returns an
  escape direction. The Schwarzschild field is `K-123`.
- `K-112` First `tsotchke/libirrep` integration per ADR-0005.
- `K-113` Expand brain corpus toward the full 700-profile roster.
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

*(empty)*

## In Progress

- `K-111` CUDA port of the ray marcher (RTX 5070 Ti). Stage 0 only:
  ADR-0013, `viz/renderer.hpp` still shades on the CPU, and
  `src/viz/cuda_probe.cu` is a device-count probe compiled only when
  `BHDS_ENABLE_CUDA=ON` and `nvcc` is present. Acceptance for the port
  itself is still open: a device image matches the CPU image.

## Review

- `K-123` Star field behind the Schwarzschild lensed disk. Escaped rays
  sample a synthetic celestial sphere at the geodesic escape azimuth.
  Acceptance: shadow center stays black, disk pixels are unchanged when
  stars are toggled, a weak ray's escape azimuth sits just above π, and
  `tests/starfield_tests.cpp` passes. Images: `docs/images/lensed_disk_i80.png`,
  `lensed_disk_i20.png`.

## Done

- `K-101` Schwarzschild null geodesic, photon sphere at \(r = 3M\), shadow
  radius \(\sqrt{27}\,M\) (`geodesics/schwarzschild_photon.hpp`,
  `tests/geodesic_tests.cpp`). Merged on main, 2026-06-13.
- `K-109` Kerr null geodesics, Carter constant (`geodesics/kerr_geodesic.hpp`).
  Merged on main, 2026-06-13.
- `K-110` CPU photon ring and lensed disk, Schwarzschild and Kerr.
  Merged on main, 2026-06-13.
- `K-114` `.gitattributes` eol rules, including LF for sources and CRLF for
  `.ps1` (ADR-0008). On main.
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
