# OPPENGROK Black Hole Simulation Lab

Research-first C++20 black hole simulation lab, building toward a **visual,
GPU-accelerated black hole simulation you can run on your own machine**.
The development target is an RTX-class consumer GPU (reference hardware:
NVIDIA RTX 5070 Ti 12 GB, Intel Core Ultra 9 275HX, 80 GB DDR5); the repo
is for anyone with that class of GPU power who wants a scientifically
honest simulation rather than a screensaver.

The path there is deliberate: analytic observables first (done), then
geodesic integration, then a ray-marched photon renderer that consumes the
validated physics kernel. Every value the simulation produces is labeled
with its scientific truth tier — beauty never outranks correctness.

This is the real local repository. Source code, research notes, audits,
tests, logs, and architecture decisions belong here and get committed
through Git. ZIP files are intake artifacts only, never the source of truth.

## Current Status

A validated analytic core with research infrastructure — not yet a GRMHD
or numerical-relativity solver, and not yet visual.

Implemented now:

- C++20 modular kernel: `core/` (constants, truth labels), `metrics/`
  (exact Schwarzschild and Kerr observables), `data/` (full-precision,
  truth-tier-labeled CSV export).
- CLI executable with `--mass`, `--spin`, `--format text|csv`, `--steps`.
- Strong physical-units header (`include/blackhole_ds/units.hpp`) with
  compile-time dimensional safety, enforced by tests.
- Brain/Soul reasoning-lens corpus: 20 XML profiles (physicists,
  mathematicians, astronomers) with XSD schema and validation.
- Research source-card corpus: 20 foundational sources (1828-2025) with
  JSON Schema, truth-tier labels, and a JSONL RAG index.
- SQLite star schema (`data/schema.sql`) and Python data-science harness.
- Deterministic corpus generators with a CI drift gate.
- Vision, mission, scientific integrity charter, ADRs, ERD and system
  diagrams, daily workflow automation, CI on GitHub Actions.

Not implemented yet (in build order):

- Geodesic integrator (next major milestone).
- Ray-marched photon renderer / visual layer (GPU; the headline goal).
- GRMHD or numerical-relativity solver.
- Truth-tier column in the SQLite schema.
- Production Power BI/Excel templates.
- Tsotchke ecosystem integrations (planned, ADR-gated; see
  `docs/integrations/`).

## Repository Layout

```text
.
|-- CMakeLists.txt                  C++20 build (CMake >= 3.20)
|-- README.md
|-- AUDIT-250-POINT-GOLD-STANDARD.md
|-- CODEOWNERS / CONTRIBUTING.md / SECURITY.md / LICENSE (AGPL-3.0)
|-- assets/diagrams/                Source diagrams (Mermaid)
|-- data/schema.sql                 Canonical SQLite star schema
|-- docs/
|   |-- architecture/               ARCHITECTURE, HIERARCHY, ERD, diagrams
|   |-- audits/                     Audit reports (see latest full review)
|   |-- integrations/               Tsotchke / Eshkol integration plans
|   |-- log/                        DAILY_LOG + DECISIONS (ADRs)
|   |-- operations/ planning/ process/ reports/ testing/
|   |-- research/                   Research program + source_cards/
|   |-- source/                     Philosophy and audit input material
|   `-- vision/                     VISION, MISSION, integrity charter
|-- external/                       Third-party adapters (ADR-gated, opt-in)
|-- include/blackhole_ds/
|   |-- units.hpp                   Strong-typed physical quantities
|   |-- core/ metrics/ data/        Kernel headers
|-- knowledge/
|   |-- brains/                     Reasoning-lens XML profiles + seeds
|   `-- papers/                     Source seeds + JSONL RAG index
|-- schemas/                        brain_soul.xsd, source_card.json
|-- scripts/
|   |-- Validate-ResearchOS.py      Umbrella validation gate
|   |-- brains/ research/           Corpus generators + validators
|   |-- dev/                        build/test/audit/Daily-Commit helpers
|   `-- local/                      Package intake (legacy)
|-- src/
|   |-- cli/main.cpp                CLI entry point
|   `-- core/ metrics/ integrators/ data/   Future module homes
|-- tests/smoke_tests.cpp           Analytic + dimensional-safety tests
`-- tools/blackhole_ds_harness.py   Python reference harness
```

## Build And Test

Windows PowerShell:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
python scripts/Validate-ResearchOS.py
```

Linux/macOS shell:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
python3 scripts/Validate-ResearchOS.py
```

Run the seed executable:

```powershell
.\build\Release\blackhole_ds.exe
```

or, depending on generator:

```powershell
.\build\blackhole_ds.exe
```

## Scientific Truth Labels

All features must declare their model status:

- `analytic_classical`: exact or standard analytic GR result.
- `numerical_approximation`: numerical method with documented error bounds.
- `observational_constraint`: anchored to published/open observational data.
- `visualization_metaphor`: explanatory visual, not a physics claim.
- `pedagogical_simplification`: teaching simplification with stated limits.
- `speculative_extension`: frontier idea, clearly separated from validated code.

The project must never mix these labels casually. A plot can be beautiful and
still be wrong; the data and validation decide.

## Development Workflow

The everyday loop is wrapped in a single script. See
[`docs/process/DAILY_WORKFLOW.md`](docs/process/DAILY_WORKFLOW.md) for the
full reference.

```powershell
.\scripts\dev\Daily-Commit.ps1 -Message "Short imperative subject" -Push
```

That script runs the validation gate, then build, then tests, and only
commits and pushes when everything passes. Skip steps you do not need
with `-SkipBuild` or `-SkipTests`.

Individual helpers:

```powershell
.\scripts\dev\build.ps1                # cmake configure + build
.\scripts\dev\test.ps1                 # ctest with output-on-failure
.\scripts\dev\audit.ps1                # validation + corpus rebuild
.\scripts\dev\clean.ps1                # wipe build/exports/_incoming
.\scripts\dev\Install-PreCommitHook.ps1   # gate every git commit
```

Manual loop:

```powershell
git status
python scripts/Validate-ResearchOS.py
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
git add -A
git commit -m "Describe the research or engineering increment"
git push origin main
```

Incoming ZIP/package workflow:

```powershell
powershell.exe -ExecutionPolicy Bypass -File .\scripts\local\Sync-OppengrokLocalRepo.ps1 `
  -PackagePath "$env:USERPROFILE\Downloads\oppengrok-blackhole-sim-v5-research-os.zip" `
  -RemoteUrl "https://github.com/0thernes/Stress-Test-Agents-Maxxxing.git" `
  -Push
```

That script archives and extracts packages under `_incoming/`, creates a backup
branch, validates, commits, rebases, and pushes only when `-Push` is supplied.

## Project Rules

- No ZIP-as-final workflow.
- No untracked project work after a meaningful change.
- No generated build/data exports committed unless explicitly curated.
- No secrets, credentials, tokens, private keys, or local daemon state.
- Research claims need a source, a truth label, and a validation path.
- Tests must grow with risk: unit tests first, then CLI/data E2E, then A/B and
  numerical benchmarks.
- Architecture decisions go in `docs/architecture/`.
- Project progress goes in `docs/reports/PROJECT_LOG.md`.

## Next Milestones

The visual simulation is the destination; these are the steps in order.

1. First geodesic integrator (null geodesics in Schwarzschild, RK45 with
   error control), validated against the analytic photon sphere.
2. Truth-tier column in `data/schema.sql` plus matching exporter and
   harness updates, so exact values and approximations can never share a
   table unlabeled.
3. E2E test: run executable, emit data, validate schema ingest round-trip.
4. A/B harness for integrator candidate comparison (error, runtime,
   stability).
5. Kerr geodesics + first lensed-image computation on CPU.
6. GPU port of the ray marcher (CUDA, RTX 5070 Ti class) - the first
   visual prototype: gravitationally lensed accretion-disk render with
   honest tier labeling (visualization_metaphor for color mapping,
   analytic/numerical tiers for the geometry underneath).
7. First tsotchke integration (libirrep) per ADR-0005.

This repo is now the place where that work happens.
