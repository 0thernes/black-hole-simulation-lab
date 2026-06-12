# Repository Hierarchy

This is the live map of the repository. Update this file when you add a
top-level directory or change ownership semantics.

```text
.
|-- .github/                           CI workflows
|   `-- workflows/ci.yml
|-- .vscode/                           VS Code workspace
|-- .oppengrok/                        Agent coordination (claim files, swarm board)
|
|-- assets/                            Source diagrams, charts, icons
|   `-- diagrams/                      Mermaid (.mmd), SVG, PlantUML sources
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
|       |-- core/                      Core types
|       |-- metrics/                   Schwarzschild, Kerr, Kerr-Newman ...
|       |-- integrators/               Geodesic integrators
|       `-- data/                      Export/import contracts
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
|   |-- BlackHoleDS.cpp                Current seed executable (will be split)
|   |-- core/                          Future: core engine
|   |-- metrics/                       Future: spacetime metrics implementations
|   |-- integrators/                   Future: geodesic integrators
|   |-- data/                          Future: CSV/JSON exporters
|   `-- cli/                           Future: CLI front end
|
|-- tests/                             CTest targets
|   `-- smoke_tests.cpp                Analytic + units smoke tests
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
