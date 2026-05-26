# Development Gameplan

This is the working gameplan for turning the starter repo into a real black-hole
simulation lab without losing scientific honesty.

## Phase 0: Repo Recovery And Baseline

- Commit all real local project files.
- Ignore nested worktrees, generated outputs, and package archives.
- Add CMake, CTest, CI, validation, architecture, research, and testing docs.
- Push a clean baseline to GitHub.

Exit criteria:

- `git status` has no untracked project files.
- `python scripts/Validate-ResearchOS.py` passes.
- CMake build and CTest pass.
- `origin/main` matches local `main`.

## Phase 1: Analytic Core

- Split analytic Schwarzschild/Kerr formulas out of the seed executable.
- Add tests for photon sphere, ISCO, horizon radius, and shadow approximations.
- Add CLI flags for mass, spin, observer inclination, and output format.
- Emit CSV and JSON with model-status labels.

Exit criteria:

- Unit tests cover analytic limits.
- CLI E2E test validates generated output.
- Docs explain formula sources and tolerances.

## Phase 2: Research Corpus

- Add curated paper/source cards for core black-hole references.
- Track each claim with source, truth label, and implementation target.
- Add validation script checks for claim-card structure.

Exit criteria:

- Every implemented formula has a source card.
- No unsupported physics claims in README or docs.

## Phase 3: Numerical Prototype

- Implement first geodesic integration prototype.
- Compare candidates through A/B harnesses.
- Record error bounds, runtime, stability, and complexity.

Exit criteria:

- A/B report chooses a default integrator with evidence.
- Regression suite catches known drift cases.

## Phase 4: Data And Visualization

- Align C++ exporter with `data/schema.sql`.
- Build first CLI-to-SQLite E2E path.
- Add first visual prototype only after the data contract is stable.

Exit criteria:

- A reproducible run can produce queryable data and a documented visual.

## Phase 5: Scale And Review

- Add CI matrix.
- Add slow/nightly science checks.
- Add review templates and audit gates.
- Prepare milestone releases with signed project logs.
