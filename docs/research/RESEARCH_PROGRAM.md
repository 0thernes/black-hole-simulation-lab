# Research Program

This project builds from stable analytic foundations toward numerical and visual
black-hole simulation. The research program is deliberately staged so the code
does not outrun the evidence.

## Foundation Topics

- Schwarzschild geometry.
- Kerr geometry.
- Horizon radius, photon sphere, ISCO, and shadow diameter approximations.
- Geodesic equations and conserved quantities.
- Numerical integration and error control.
- Observational constraints from horizon-scale imaging and gravitational waves.
- Data contracts for reproducible simulation runs.

## Source Card Pattern

Each research source should become a small source card with:

- bibliographic citation or URL
- claim summary
- truth label
- equations or observables used
- implementation target
- validation/test target

Suggested path:

```text
docs/research/source_cards/YYYY-short-title.md
```

## Claim Handling

Claims enter the repo in three steps:

1. Research note: source-backed, no code required.
2. Implementation ticket: maps claim to module and tests.
3. Validated code: claim has source, implementation, and regression coverage.

## Current Research Priority

1. Lock analytic Schwarzschild/Kerr formula references.
2. Define tolerances for the seed observable calculations.
3. Build the first source cards.
4. Add data and E2E tests for generated observables.
5. Only then expand to numerical geodesics and visual simulation.
