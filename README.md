# OPPENGROK Black Hole Simulation Lab

Research-first C++ black hole simulation workspace for building a scientifically
auditable lab: analytic black-hole observables, future geodesic integration,
data exports, visualization, A/B experiments, E2E validation, and review gates.

This is the real local repository. Source code, research notes, audits, tests,
logs, and architecture decisions belong here and get committed through Git.
ZIP files are intake artifacts only, never the source of truth.

## Current Status

This repo is a serious starter baseline, not a completed GRMHD or numerical
relativity solver.

Implemented now:

- C++ seed executable for analytic Schwarzschild/Kerr observables.
- Strong physical-units header foundation in `include/blackhole_ds/units.hpp`.
- SQLite research/data schema in `data/schema.sql`.
- Python data-science harness in `tools/blackhole_ds_harness.py`.
- Local package intake workflow in `scripts/local/Sync-OppengrokLocalRepo.ps1`.
- Research, architecture, testing, planning, audit, and project-log documents.
- CMake, CTest, CI, and local Research OS validation scaffolding.

Not implemented yet:

- Full geodesic integrator.
- GRMHD or numerical-relativity solver.
- Photon-ring renderer or interactive visual layer.
- Production Power BI/Excel templates.
- Large research corpus/RAG index.
- Agent automation that runs without human review.

## Repository Layout

```text
.
|-- CMakeLists.txt
|-- README.md
|-- AUDIT-250-POINT-GOLD-STANDARD.md
|-- data/
|   `-- schema.sql
|-- docs/
|   |-- architecture/
|   |-- audits/
|   |-- operations/
|   |-- planning/
|   |-- process/
|   |-- reports/
|   |-- research/
|   |-- source/
|   `-- testing/
|-- include/
|   `-- blackhole_ds/
|-- scripts/
|   |-- local/
|   `-- Validate-ResearchOS.py
|-- src/
|-- tests/
`-- tools/
```

`docs/source/` contains source philosophy and audit input material. It is kept
out of the root so the root stays buildable and easy to review.

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

1. Split the current C++ seed into `core`, `metrics`, and `data` modules.
2. Add a real analytic test suite for Schwarzschild and Kerr edge cases.
3. Add CLI flags and machine-readable CSV/JSON output.
4. Add first E2E test: run executable, emit data, validate schema ingest.
5. Add curated paper/source cards under `docs/research/`.
6. Add A/B harness for integrator candidate comparison.
7. Add visual prototype after the physics/data contracts are stable.

This repo is now the place where that work happens.
