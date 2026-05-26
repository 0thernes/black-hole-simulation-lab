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

## Build Contract

The root `CMakeLists.txt` defines:

- `blackhole_ds`: current seed executable.
- `blackhole_ds_smoke_tests`: analytic and type-safety smoke tests.
- CTest entries for both.

Future modules should be added as libraries before new executables are added.

## Future Target Shape

```text
include/blackhole_ds/
|-- core/
|-- metrics/
|-- integrators/
|-- chaos/
|-- data/
`-- viz/

src/
|-- core/
|-- metrics/
|-- integrators/
|-- data/
`-- main.cpp
```

The first refactor should split `src/BlackHoleDS.cpp` into small modules while
preserving test behavior.
