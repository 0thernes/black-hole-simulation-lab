# Architecture

OPPENGROK Black Hole Simulation Lab is organized around a research-to-code
pipeline. The project starts with analytic black-hole observables and grows
toward validated numerical modules, visualization, and data-science workflows.

## Module Boundaries

- `include/blackhole_ds/`: public C++ headers and stable contracts.
- `src/`: executable and compiled implementation.
- `tests/`: unit and smoke tests run by CTest.
- `tools/`: research/data-science helper tools.
- `data/`: schemas and curated seed/reference data.
- `docs/research/`: source-backed research program and source cards.
- `docs/testing/`: unit, integration, E2E, A/B, and audit test plans.
- `docs/operations/`: local repo, GitHub, and package intake operations.

## Truth Labels

Every model, visualization, and exported value must be tagged with one of:

- `analytic_classical`
- `numerical_approximation`
- `observational_constraint`
- `visualization_metaphor`
- `pedagogical_simplification`
- `speculative_extension`

Truth labels prevent the project from presenting frontier ideas, approximations,
and validated classical formulas as though they were the same kind of claim.

## Data Contract

The canonical data contract begins in `data/schema.sql`.

Rules:

- Every run has a seed, model status, parameters, and source commit.
- Derived observables are reproducible from code and inputs.
- Visualization consumes simulation state; it does not own physics.
- Python harness output and future C++ exporter output must converge on the same
  schema and validation tolerances.

## Numerical Integrators

`include/blackhole_ds/integrators/` provides the ODE machinery the geodesic
solver will use:

- `ode_state.hpp`: fixed-size, stack-allocated `State<N>` (`std::array`)
  with `add`/`axpy`/`scale` (all O(N)) and a tolerance-weighted RMS error
  norm for the adaptive controller.
- `rk4.hpp`: classic fixed-step fourth-order Runge-Kutta. Global error
  O(h^4); four derivative evaluations per step.
- `rk45.hpp`: adaptive Dormand-Prince 5(4) ("ode45"). A 5th-order solution
  with an embedded 4th-order error estimate drives step-size control, so
  the integrator concentrates work where the solution is sensitive (e.g.
  near the photon sphere). FSAL reuse: six derivative evaluations per
  accepted step.

Both are header-only, allocation-free in the hot loop, and templated on the
state dimension N (geodesics will use N=8: four position + four momentum
components). Convergence order and adaptive behavior are verified in
`tests/integrator_tests.cpp`. Complexity is documented in
`docs/engineering/COMPLEXITY.md`.

These are tagged `numerical_approximation`: every result carries a
documented error order, never presented as exact.

## Build Contract

The root `CMakeLists.txt` defines:

- `blackhole_ds`: CLI executable (`src/cli/main.cpp` + shim).
- `blackhole_ds_smoke_tests`: analytic and type-safety smoke tests.
- `blackhole_ds_integrator_tests`: ODE-integrator convergence tests.
- CTest entries for all three, plus a smoke run of the executable.

Future modules should be added as libraries before new executables are added.

## Future Target Shape

```text
include/blackhole_ds/
|-- core/          (done: constants, truth_label)
|-- metrics/       (done: schwarzschild, kerr)
|-- integrators/   (done: ode_state, rk4, rk45)
|-- chaos/         (planned: Lyapunov, Poincare sections)
|-- data/          (done: csv_writer; planned: json, sqlite)
`-- viz/           (planned: ray marcher, photon ring)

src/
|-- cli/main.cpp   (done)
|-- core/ metrics/ integrators/ data/   (compiled units as they grow)
```

`src/BlackHoleDS.cpp` is now an empty shim; the program lives in
`src/cli/main.cpp`. The next compiled units are the geodesic right-hand
side (a `metrics/`-driven derivative functor) feeding `rk45_integrate`.
