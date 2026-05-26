# Changelog

All notable project changes should be recorded here. Keep this human-readable;
use `docs/reports/PROJECT_LOG.md` for detailed operational notes.

## 2026-05-26 (Iteration 2: Repo skeleton for daily work)

- Added the module directory skeleton matching the architecture doc:
  `src/{core,metrics,integrators,data,cli}/` plus matching headers under
  `include/blackhole_ds/`.
- Added `knowledge/brains/{mathematicians,physicists,astronomers,coders,
  developers,engineers,architects,scientists}/` to hold the seven 100-strong
  reasoning-lens XML rosters that arrive in Iteration 4.
- Added `knowledge/papers/`, `schemas/`, `external/`, `assets/diagrams/`,
  `docs/{integrations,log,vision,research/source_cards}/`,
  `scripts/{dev,brains,research}/`, and `docs/architecture/diagrams/`.
- Added governance files: `CODEOWNERS`, `CONTRIBUTING.md`, `SECURITY.md`.
- Added `docs/log/DAILY_LOG.md` (operational story) and
  `docs/log/DECISIONS.md` (architecture decision records with ADRs 0001-0004).
- Added `docs/architecture/HIERARCHY.md` describing the live directory map
  and ownership semantics.
- Added VS Code workspace under `.vscode/`: settings, recommended extensions,
  build/test tasks for both g++ (immediate) and CMake (after install), and
  debugging configs.

## 2026-05-26 (Iteration 1: Foundation sanity)

- Rewrote `src/BlackHoleDS.cpp` to use the canonical `blackhole_ds::units`
  types from `include/blackhole_ds/units.hpp` instead of redefining its own
  `Length`, `Time`, and `Dimensionless` structs. Eliminates a quiet type-system
  divergence between the executable and the smoke tests.
- Replaced the chatty seed banner with a professional, sober output that still
  emits scientifically-honest `model_status` labels for every printed table.
- Rewrote `.gitignore` to use whole-name patterns for secret detection instead
  of substring matches like `*token*` and `*auth*` that would block legitimate
  filenames (`token_parser.hpp`, `authentication.cpp`). Allow-listed curated
  research data and diagrams that must stay tracked.
- Verified `g++ 14.2` (MinGW UCRT, via CodeBlocks) builds both targets and that
  smoke tests pass. CMake install is queued.

## 2026-05-26

- Established the local Git repository as the source of truth.
- Added direct local package intake workflow and hardened package selection.
- Added CMake, CTest, CI, and Research OS validation scaffolding.
- Added architecture, research, testing, workflow, gameplan, and project-log docs.
- Organized source philosophy/audit inputs under `docs/source/`.
- Added the existing C++ seed, SQLite schema, Python harness, and research docs to
  the committed project baseline.
