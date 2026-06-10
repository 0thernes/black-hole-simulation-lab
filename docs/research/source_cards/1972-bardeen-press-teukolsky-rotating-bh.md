# Source Card: Rotating black holes: Locally nonrotating frames, energy extraction, and scalar synchrotron radiation

- **Slug:** `1972-bardeen-press-teukolsky-rotating-bh`
- **Kind:** `paper`
- **Authors:** James M. Bardeen, William H. Press, Saul A. Teukolsky
- **Year:** 1972
- **Venue:** Astrophysical Journal
- **Model status:** `analytic_classical`

## Identifier

- DOI: `10.1086/151796`

## Summary

Definitive analytic treatment of equatorial geodesics in Kerr. Source of the closed-form expressions for ISCO and photon-sphere radii.

## Claims

- **Statement:** Prograde Kerr ISCO in geometric units satisfies r_ISCO = 3 + Z2 - sign(a)*sqrt((3 - Z1)(3 + Z1 + 2 Z2)).
  - Tier: `analytic_classical`
  - Used in: `src/BlackHoleDS.cpp`, `tests/smoke_tests.cpp`

- **Statement:** Schwarzschild limit a -> 0 gives r_ISCO = 6.0 exactly.
  - Tier: `analytic_classical`
  - Used in: `tests/smoke_tests.cpp`

## Related Brain Profiles

- [`subrahmanyan-chandrasekhar`](../../../knowledge/brains/INDEX.md)
- [`kip-thorne`](../../../knowledge/brains/INDEX.md)

## Metadata

- Card version: 1.0.0
- Created: 2026-05-26
- Updated: 2026-05-26
- Author: blackhole_ds-seed-corpus
- Confidence: seed
