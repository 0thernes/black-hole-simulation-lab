# Changelog

All notable project changes should be recorded here. Keep this human-readable;
use `docs/reports/PROJECT_LOG.md` for detailed operational notes.

## 2026-05-26 (Iteration 4: Brain/Soul XML system)

- `schemas/brain_soul.xsd`: XML Schema for the reasoning-lens profiles.
  Enforces required sections (identity, contributions, reasoning_lens,
  how_to_apply, metadata), the category enumeration, and the truth-tier
  enumeration for bibliography model statuses.
- `knowledge/brains/seed_profiles.json`: source of truth for the seed
  roster. 20 deep profiles total: 9 physicists (Schwarzschild, Kerr,
  Einstein, Penrose, Hawking, Thorne, Newman, Bekenstein, Strominger),
  6 mathematicians (Riemann, Poincare, Gauss, Cartan, Choquet-Bruhat, Yau),
  5 astronomers (Chandrasekhar, Eddington, Ghez, Genzel, Doeleman). Each
  profile carries real contributions, year, relevance to BlackHoleDS,
  proof standard, favorite questions, failure modes, bibliography with
  truth-tier labels, and module-relevance routing.
- `scripts/brains/build_brains.py`: deterministic generator that emits one
  XML file per profile under `knowledge/brains/<category>s/<slug>.xml`,
  plus `knowledge/brains/MANIFEST.json` and `knowledge/brains/INDEX.md`.
- `scripts/brains/validate_brains.py`: structural validator. Checks
  parseability, schema location, slug-filename match, category-directory
  match, required sections, and INDEX.md count consistency.
- `scripts/Validate-ResearchOS.py`: now runs the brain validator
  automatically if the manifest exists. Allows staged adoption.
- Generated 20 XML profiles, MANIFEST.json, and INDEX.md. Validation passes.

## 2026-05-26 (Iteration 3: Vision, mission, and ERM documents)

- `docs/vision/VISION.md`: long-term aim, what the project is, what it is
  not, and a three-year horizon.
- `docs/vision/MISSION.md`: 12-month mission with six pillars and quarterly
  objectives for Q1 through Q4.
- `docs/vision/SCIENTIFIC_INTEGRITY_CHARTER.md`: project constitution with
  the six truth tiers (analytic_classical, numerical_approximation,
  observational_constraint, visualization_metaphor, pedagogical_simplification,
  speculative_extension), source discipline rules, units and numerics rules,
  reproducibility rules, and AI-assisted code policy.
- `docs/architecture/ERD.md` + `docs/architecture/diagrams/erd.mmd`: human
  and Mermaid views of the canonical SQLite star schema.
- `docs/architecture/SYSTEM_DIAGRAM.md` +
  `docs/architecture/diagrams/system_pipeline.mmd`: end-to-end pipeline
  from research layer through contracts, kernel, data, visualization, and
  audit.
- `docs/architecture/diagrams/module_dependency.mmd`: target C++ module
  dependency graph for after Iteration 7.

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
