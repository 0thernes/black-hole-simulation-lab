# Source Card: Uber das Gravitationsfeld eines Massenpunktes nach der Einsteinschen Theorie

- **Slug:** `1916-schwarzschild-solution`
- **Kind:** `paper`
- **Authors:** Karl Schwarzschild
- **Year:** 1916
- **Venue:** Sitzungsberichte der Preussischen Akademie der Wissenschaften
- **Model status:** `analytic_classical`

## Identifier

- URL: https://articles.adsabs.harvard.edu/full/1916SPAW.......189S

## Summary

First exact non-trivial solution to Einstein's vacuum field equations. Describes the spacetime geometry outside a non-rotating, uncharged, spherically symmetric mass.

## Claims

- **Statement:** Outside a spherically symmetric mass M, in vacuum, the metric takes the Schwarzschild form.
  - Tier: `analytic_classical`
  - Equation: `ds^2 = -(1 - 2GM/(rc^2)) c^2 dt^2 + (1 - 2GM/(rc^2))^{-1} dr^2 + r^2 (d\theta^2 + \sin^2\theta d\phi^2)`
  - Used in: `include/blackhole_ds/units.hpp`, `src/BlackHoleDS.cpp`, `tests/smoke_tests.cpp`

- **Statement:** The Schwarzschild radius is r_s = 2GM/c^2.
  - Tier: `analytic_classical`
  - Equation: `r_s = 2GM/c^2`
  - Used in: `src/BlackHoleDS.cpp`

## Related Brain Profiles

- [`karl-schwarzschild`](../../../knowledge/brains/INDEX.md)
- [`albert-einstein`](../../../knowledge/brains/INDEX.md)

## Metadata

- Card version: 1.0.0
- Created: 2026-05-26
- Updated: 2026-05-26
- Author: blackhole_ds-seed-corpus
- Confidence: seed
