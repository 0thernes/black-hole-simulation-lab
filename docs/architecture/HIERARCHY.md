# Repository Hierarchy

This is the live map of the repository. Update this file when you add a
top-level directory or change ownership semantics.

```text
.
|-- .github/                           CI workflows + Dependabot
|   |-- workflows/ci.yml
|   `-- dependabot.yml
|-- .vscode/                           VS Code workspace
|
|-- assets/                            Reserved for charts/icons (empty today;
|   `-- diagrams/                      architecture .mmd live under docs/)
|
|-- data/                              Curated data and schema
|   `-- schema.sql                     Canonical SQLite star schema
|
|-- docs/                              All documentation
|   |-- architecture/                  System design, ERD, hierarchy
|   |   |-- ARCHITECTURE.md            Module boundaries, truth labels, contracts
|   |   |-- HIERARCHY.md               This file
|   |   `-- diagrams/                  Architecture-specific diagrams
|   |-- audits/                        Audit reports
|   |-- integrations/                  Third-party integration plans (tsotchke etc.)
|   |-- log/
|   |   |-- DAILY_LOG.md               Append-only daily entries
|   |   `-- DECISIONS.md               Architecture decision records (ADRs)
|   |-- operations/                    Local repo + GitHub operations
|   |-- planning/                      Development gameplan, roadmaps
|   |-- process/                       Workflow, review process
|   |-- reports/                       Engineering log, status reports
|   |-- research/                      Research program, source cards
|   |   `-- source_cards/              One file per cited paper
|   |-- source/                        Philosophy and audit input material
|   |-- testing/                       Test strategy
|   `-- vision/                        Vision, mission, integrity charter
|
|-- external/                          Third-party deps (submodules, FetchContent)
|
|-- include/                           Public C++ headers
|   `-- blackhole_ds/
|       |-- units.hpp                  Strong-typed physical quantities (units, validators)
|       |-- core/                      constants.hpp, truth_label.hpp
|       |-- metrics/                   schwarzschild.hpp, kerr.hpp
|       |-- integrators/               ode_state.hpp, rk4.hpp, rk45.hpp (DP 5(4))
|       |-- geodesics/                  schwarzschild_photon.hpp (light bending)
|       |-- viz/                         ascii_shadow.hpp (first image)
|       |-- cli/                        options.hpp (testable arg parser)
|       `-- data/                      csv_writer.hpp (export contracts)
|
|-- knowledge/                         Research knowledge corpus
|   |-- brains/                        Scientist reasoning-lens XML profiles
|   |   |-- mathematicians/
|   |   |-- physicists/
|   |   |-- astronomers/
|   |   |-- coders/
|   |   |-- developers/
|   |   |-- engineers/
|   |   |-- architects/
|   |   `-- scientists/
|   `-- papers/                        Paper/source index (RAG layer)
|
|-- schemas/                           Schema definitions (SQL, XSD, JSON Schema)
|
|-- scripts/                           Build / dev / research helpers
|   |-- Validate-ResearchOS.py         Repo invariants gate
|   |-- dev/                           Daily workflow helpers
|   |-- brains/                        Brain XML builders/validators
|   |-- research/                      Source-card builders/validators
|   `-- local/                         Local sync scripts
|
|-- src/                               C++ implementation
|   |-- BlackHoleDS.cpp                Empty shim (program moved to cli/main.cpp)
|   |-- cli/main.cpp                   CLI entry point (done)
|   |-- core/ metrics/ integrators/ data/   Future compiled units (headers
|   |                                  are header-only today)
|
|-- tests/                             CTest targets
|   |-- smoke_tests.cpp                Analytic + dimensional-safety tests
|   `-- integrator_tests.cpp           ODE convergence + adaptive-step tests
|
|-- tools/                             Standalone Python tools
|   `-- blackhole_ds_harness.py        Reference data-science harness
|
|-- AUDIT-250-POINT-GOLD-STANDARD.md   Living audit entrypoint
|-- CHANGELOG.md                       Project changelog
|-- CMakeLists.txt                     Build configuration
|-- CODEOWNERS                         GitHub code ownership
|-- CONTRIBUTING.md                    Contribution rules
|-- LICENSE                            AGPL-3.0 (see ADR-0006 for rationale)
|-- README.md                          Project entrypoint
`-- SECURITY.md                        Security policy
```

## Ownership Semantics

- `include/blackhole_ds/units.hpp` is the foundation. Changes here cascade
  to every numerical module. Treat any PR touching it as a senior review.
- `data/schema.sql` is the data contract. Schema changes require an ADR and
  must be matched by C++ exporter + Python harness updates.
- `docs/architecture/`, `AUDIT-250-POINT-GOLD-STANDARD.md`, and
  `docs/log/DECISIONS.md` are the governance layer.
- `knowledge/brains/` and `knowledge/papers/` are the research corpus.
  Treat their schemas (in `schemas/`) as contracts.
