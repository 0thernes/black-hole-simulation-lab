<!-- SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved -->
<!-- Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved. -->

# M5 — Kerr shadow asymmetry vs. spin (documented comparison)

This note closes the M5 exit criterion *"shadow asymmetry vs spin matches
published EHT-style curves qualitatively, with the comparison documented."* It
quantifies the shadow observables of a Kerr black hole as a function of spin
`a/M` and observer inclination `i`, computed from the exact closed-form
Bardeen (1973) boundary — the same physics implemented in
[`geodesics/kerr_shadow.hpp`](../../include/blackhole_ds/geodesics/kerr_shadow.hpp)
and used by the `--kerr-shadow` / `--kerr-disk` renders.

Reproduce with:

```
python scripts/research/kerr_shadow_asymmetry.py
```

**Truth tier:** `analytic_classical`. The shadow boundary is the projection of
the unstable spherical photon orbits; it is closed-form and involves no
numerical geodesic integration. The script asserts the exact anchors below on
every run.

## Definitions (units of `GM/c²`, `M = 1`)

- **width** — horizontal (α) extent of the shadow; the axis carrying the spin
  asymmetry (perpendicular to the projected spin axis).
- **height** — vertical (β) extent; along the projected spin axis.
- **displacement** — `(α_max + α_min)/2`, the offset of the shadow's α-centroid
  from the line of sight. Zero for Schwarzschild; the defining spin signature.
- **d_eq** — `√(width·height)`, a single size scale.
- **frac_asym** — `|displacement| / (¼(width+height))`, a dimensionless
  asymmetry.

## Exact validation anchors (checked by `assert` in the script)

| Quantity | Computed | Exact | Status |
|---|---|---|---|
| Shadow diameter at `a=0` (any `i`) | 10.3923 | `2√27 = 10.3923` | ✅ |
| Displacement at `a=0` | 0 | 0 | ✅ |
| Prograde equatorial photon radius at `a=1` | 1.0000 | 1 | ✅ |
| Retrograde equatorial photon radius at `a=1` | 4.0000 | 4 | ✅ |

## Edge-on (`i = 89.9°`) — maximal spin signature

| a/M | width | height | d_eq | displacement | frac_asym | α range |
|----:|------:|-------:|-----:|-------------:|----------:|:--------|
| 0.000 | 10.392 | 10.392 | 10.392 | +0.000 | 0.000 | [−5.20, +5.20] |
| 0.300 | 10.334 | 10.392 | 10.363 | +0.604 | 0.117 | [−4.56, +5.77] |
| 0.600 | 10.149 | 10.392 | 10.270 | +1.238 | 0.241 | [−3.84, +6.31] |
| 0.900 |  9.671 | 10.392 | 10.025 | +1.993 | 0.397 | [−2.84, +6.83] |
| 0.990 |  9.230 | 10.392 |  9.794 | +2.364 | 0.482 | [−2.25, +6.98] |
| 0.998 |  9.102 | 10.392 |  9.726 | +2.441 | 0.501 | [−2.11, +6.99] |

At extremal spin edge-on the boundary spans α ∈ [−2.11, +6.99] — i.e. a flat
prograde edge near `|α| ≈ 2 M` and a rounded retrograde edge near `|α| ≈ 7 M`,
the classic D-shape of Bardeen (1973) and Chandrasekhar (1983), displaced by
≈ 2.4 M. The vertical extent stays fixed at `2√27` because the β-turning
points are set by the polar photon orbits, which are spin-insensitive.

## M87*-like inclination (`i = 17°`)

| a/M | width | height | d_eq | displacement | frac_asym |
|----:|------:|-------:|-----:|-------------:|----------:|
| 0.000 | 10.392 | 10.392 | 10.392 | +0.000 | 0.000 |
| 0.500 | 10.230 | 10.255 | 10.242 | +0.301 | 0.059 |
| 0.900 |  9.804 |  9.890 |  9.847 | +0.600 | 0.122 |
| 0.940 |  9.724 |  9.834 |  9.779 | +0.638 | 0.130 |
| 0.998 |  9.591 |  9.742 |  9.667 | +0.705 | 0.146 |

## Comparison to the literature

1. **Shadow size is weakly spin-dependent.** The equivalent diameter shrinks
   only −6.4% from `a=0` to `a=0.998` edge-on. This matches the well-known
   result that the Kerr shadow's *area/size* varies by `<~7%` across the full
   spin range (Bardeen 1973; Takahashi 2004), which is why shadow *size* alone
   is a poor spin estimator and the EHT's M87* size measurement is consistent
   with a broad spin range.
2. **The asymmetry is the spin signature, and it is inclination-suppressed.**
   The displacement grows monotonically with `a` and scales with `sin i`
   (≈ `sin i` at fixed `a`, exact via `α = −ξ/sin i`). At M87*'s near-face-on
   `i ≈ 17°` the fractional asymmetry stays `≲ 0.15` even at extremal spin —
   consistent with why shadow-asymmetry alone weakly constrains M87* spin,
   whereas a high-inclination source (e.g. an edge-on disk) would show a
   pronounced D-shape.
3. **Exact limits.** The `a → 0` circle (`2√27`) and the `a = 1` equatorial
   photon radii (1 and 4) are reproduced to machine precision.

## What remains (honest scope)

This is the *geometric* shadow asymmetry. A fully quantitative match to a
specific EHT measurement (e.g. M87*'s `42 ± 3 µas` ring diameter) additionally
requires the mass-to-distance scaling and the emission model; the renderer's
brightness profile is a `visualization_metaphor`, not a radiometric GRMHD
model. Those belong to a later milestone.
