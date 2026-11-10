# Physics implemented in this tree

This file is the list of equations the code actually evaluates, where they
come from, how they were checked, and what they are not. Truth tiers are
the ones in `include/blackhole_ds/core/truth_label.hpp`.

## Languages

The simulator is written in C, C++, and CUDA C++. Python is for the
harness, analysis, and validation only. CUDA is optional
(`BHDS_ENABLE_CUDA`, off by default) and the image path stays on the CPU
when `nvcc` is absent (ADR-0013).

Eshkol is in the allowed set. It is a separate compiler, not a header in
this build. See the toolchain section.

JavaScript, TypeScript, HTML, and WebGL are not used for new code. The
existing `gh-pages` branch is left untouched.

## Toolchain, read before linking

Each library was read from its README and public API on 2026-11-10. Pins
are the `0thernes` forks. A library is linked only where that API does the
job. The others are recorded so they are not forced in later under a
different name.

| Library | Pin | What it actually is | Used here? |
|---|---|---|---|
| Eshkol | `0thernes/eshkol` @ `d2f6a32ff2f9c42072dd26849f0f299fd59e6c64`, parent `tsotchke/eshkol` | A Scheme compiler. Automatic differentiation is the form `(derivative f x)` inside that compiler. There is no C API that returns a Christoffel symbol. | No. `eshkol` is not on this machine and not on the Windows CI image. Metric derivatives are finite-differenced in C++ (`schwarzschild_christoffel.hpp`). An unrun `.esk` file would not be a check. |
| quantum_rng | `0thernes/quantum_rng` @ `1a77e77f803c63883349b658361c06401cd8ceb7`, parent `tsotchke/quantum_rng` | C library. `qrng_init` with a non-empty seed and `qrng_double` are a deterministic stream. The headers need `unistd.h`. | Yes, for seeded samples of the Page–Thorne flux, and only when `unistd.h` exists. MSVC falls back to a local splitmix mixer. The log line says which one ran. See `external/quantum_rng/PROVENANCE.md`. |
| Moonlab | `0thernes/moonlab` @ `cd3b2347e3739327548708073d7e4ac710cc6a6c`, parent `tsotchke/moonlab` | Quantum state-vector simulator (gates, Born sampling, annealing, VQE). C ABI 0.8.0. | No. It does not integrate a Kerr geodesic or a thin-disk flux. |
| QGTL | `0thernes/quantum_geometric_tensor` @ `770994aa8ddd858e6bb16eebb25250a7b9f67c96`, parent `tsotchke/quantum_geometric_tensor` | Fubini–Study metric and Berry curvature of a parameterized quantum state, `Q_μν = ⟨∂_μψ\|∂_νψ⟩ − ⟨∂_μψ\|ψ⟩⟨ψ\|∂_νψ⟩`. | No. That is the geometry of a quantum state, not the spacetime metric. |
| Rosette Wire | `0thernes/rosette-wire` @ `bb34bbe7536f2ea9829dc302c6a578e9fdd877a0`, parent `tsotchke/rosette-wire`, which forks `RichardHoekstra/rosette-wire` | Common Lisp (SBCL) protocol: components, receipts, verifiers. The worked example is `factorial(6) = 720`. Adapters exist for Eshkol and Moonlab. | No. It does not evaluate these observables, and SBCL is not a CI requirement. The checks are CTest. |

## What is GR, what is a model, what is not claimed

- **Established GR.** Schwarzschild and Kerr circular-orbit constants, the
  photon sphere, the critical impact parameter, the Christoffel symbols of
  the Schwarzschild metric, the vacuum invariant `I_ν / ν^3`, and the
  Lyapunov exponent of the photon sphere.
- **Model assumption.** A Novikov–Thorne thin disk: negligible thickness,
  equatorial circular geodesics, torque vanishing at the ISCO, and the
  Page–Thorne flux that follows. Lambert's law `I_em = F / π` is a further
  assumption about the emitter. Neither is a solution of the fluid
  equations, and neither includes magnetic fields.
- **Not claimed.** Covariant radiative transfer through an atmosphere.
  Elliptic-integral Kerr geodesics in the sense of Gralla & Lupsasca. A
  Kerr-disk escape direction (the Kerr tracer still returns a dark floor
  when a ray leaves). Plasma, hair, or a dark-matter halo in the default
  ray. Anything in the quantum-gravity section of the research feed.

The rendered disk colour is still the radial ramp in `disk_image.hpp`.
That ramp is a `visualization_metaphor`. It is not the Page–Thorne flux.
`g^4` in that renderer is the GR factor applied to the ramp.

## Equations

Geometric units, `M = 1`, unless a function takes a mass in solar masses.

### Schwarzschild photon sphere and shadow

Orbit equation, `u = 1/r`:

```text
d²u/dφ² = 3 u² - u
```

At `u = 1/3` the right-hand side is zero (`r = 3`). Its `u` derivative is
`6 u - 1 = 1`, so a perturbation with zero initial slope grows as
`cosh φ`.

Critical potential `u² - 2 u³` at `u = 1/3` equals `1/27`, so the critical
impact parameter satisfies `b² = 27` and `b = sqrt(27)`.

| Result | Reference value | Where it is checked |
|---|---|---|
| Photon sphere | `r = 3` | Integer certificate; `photon_sphere_dimensionless(0) = 3`; orbit RHS |
| Shadow radius | `sqrt(27)` | `b² = 27` exactly in integers; `geodesic_tests`, `shadow_tests`, `render_tests` (image radius within one pixel) |
| ISCO | `r = 6` | BPT formula at `a = 0` is the integer 6; `isco_dimensionless(0) == 6` |
| ISCO energy | `E² = 8/9` | Integer cross-multiply; `circular_energy(6, 0)` agrees to `1e-12` |
| ISCO angular momentum | `L² = 12` | Integer identity; `circular_angmom(6, 0) = 2 sqrt(3)` to `1e-12` |
| Efficiency | `1 - sqrt(8/9) ≈ 0.057190958` | `radiative_efficiency(0)` |

Source of the orbit equation and `b = sqrt(27) M`: Misner, Thorne & Wheeler
1973, chapter 25. ISCO energy and angular momentum: Bardeen, Press &
Teukolsky 1972, ApJ 178, 347, doi:10.1086/151796.

### Kerr ISCO and equatorial photon orbit

Prograde ISCO (BPT), with `a` clamped to `[-1, 1]`:

```text
Z1 = 1 + (1 - a²)^{1/3} ((1 + a)^{1/3} + (1 - a)^{1/3})
Z2 = sqrt(3 a² + Z1²)
r_isco = 3 + Z2 - sign(a) sqrt((3 - Z1) (3 + Z1 + 2 Z2))
```

Equatorial photon orbit:

```text
r_ph = 2 (1 + cos((2/3) acos(-a)))
```

`a = 0` gives `r_isco = 6` and `r_ph = 3` exactly in the floating-point
evaluation as well as in the integer reduction. `a = +1` gives both radii
equal to 1 in the closed form (extremal prograde). Negative `a` is the
retrograde branch of the same formulae.

Sensitivity, not a series coefficient from a paper: a one-sided difference
of `r_isco` at `a = 0+` is `-3.266025` for step `1e-4` and `-3.266375` for
step `1e-3`. They agree to better than `0.1%`. The slope is negative:
prograde spin moves the ISCO inward.

Code: `include/blackhole_ds/metrics/kerr.hpp`.

### Circular-orbit constants

```text
Ω = 1 / (r^{3/2} + a)
E = (r^{3/2} - 2 r^{1/2} + a) / (r^{3/4} sqrt(r^{3/2} - 3 r^{1/2} + 2 a))
L = (r² - 2 a r^{1/2} + a²) / (r^{3/4} sqrt(r^{3/2} - 3 r^{1/2} + 2 a))
```

On Schwarzschild, `E - Ω L = sqrt(1 - 3/r)`. Checked at `r = 10` to
`1e-12`. The formulae are `0/0` exactly at the ISCO; callers use the known
Schwarzschild limits or evaluate slightly outside.

Code: `include/blackhole_ds/disk/novikov_thorne.hpp`.

### Page–Thorne flux

One face, rest-mass accretion rate `Mdot`:

```text
F(r) = (Mdot / 4π) (-dΩ/dr) / (E - Ω L)²
       ∫_{r_isco}^{r} (E - Ω L) (dL/dr) dr
```

`F(r_isco) = 0`. Source: Page & Thorne 1974, ApJ 191, 499,
doi:10.1086/152990, under the Novikov–Thorne assumptions (Novikov & Thorne
1973).

For `a = 0` the integral is elementary. With `u = sqrt(r)`:

```text
∫_6^r (s - 6) / (2 sqrt(s) (s - 3)) ds
    = [u - (sqrt(3)/2) ln((u - sqrt(3))/(u + sqrt(3)))]
      - (the same at u = sqrt(6))
```

A 4000-panel Simpson rule reproduces that antiderivative at `r = 10` to
`1e-10` relative. The flux values, `Mdot = 1`:

| `r` | `F` |
|---|---|
| 10 | `1.356875463805e-4` |
| 20 | `9.450288131510e-5` |

Doubling `Mdot` doubles `F`.

For `a ≠ 0` the derivatives are complex-step derivatives (`h = 1e-8`) and
the integral is Simpson's rule, starting `1e-6` outside the ISCO because
the closed-form `E` and `L` are singular on the ISCO itself. At `a = 0.5`,
`r_isco = 4.233003`. `F(r_isco) = 0`. `F(r_isco + 4) = 3.014751698522e-4`
with 4000 panels and `3.014751698519e-4` with 1000 panels.

This is not radiative transfer. There is no optical depth, no frequency
grid, and no atmosphere.

### Intensity

Vacuum invariant: `I_ν / ν³` is constant along a ray. Integrated over
frequency for an isotropic rest-frame emitter, `I_obs = g⁴ I_em`. The code
sets `I_em = F / π` (Lambert) and returns

```text
I_obs = g⁴ F / π
```

Checked: `g = 1` returns `F/π`; `g = 2` multiplies by 16; `F = 0` returns
0. The factor `g` is an input. The renderer's own `g` is the circular-orbit
redshift in `disk_image.hpp` (Schwarzschild) and `kerr_disk_image.hpp`
(Kerr), cited there to Cunningham 1975.

### Photon-orbit Lyapunov exponent

Schwarzschild, `M = 1`, `E = 1`:

| Quantity | Value | Source |
|---|---|---|
| Coordinate-time rate `γ_t` | `1/(3 sqrt(3)) = 1/sqrt(27)` | Hadar, Kapec, Lupsasca & Strominger 2022, arXiv:2205.05064, eq. (2.12) |
| Affine-time rate | `1/sqrt(3) = 3 γ_t` | `dt/dλ = 3` on `r = 3` |
| Half-orbit exponent | `π` | Johnson et al. 2020, arXiv:1907.04329. Orbital frequency equals `γ_t`, so half an orbit accumulates `π` |
| Subring demagnification | `e^{-π} = 4.321391826377e-2` | Same paper, flux ratio of successive subrings |

Numerical check: start at `u = 1/3 + 1e-8` with `du/dφ = 0`, integrate the
orbit equation with RK4, step `1e-5`, to `φ = 0.5`. The perturbation is
`1.127620746777e-8`; `1e-8 cosh(0.5)` is `1.127620754310e-8` (relative
difference about `7e-9`).

Code: `include/blackhole_ds/geodesics/photon_lyapunov.hpp`.

### Schwarzschild Christoffel symbol

```text
Γ^r_tt = (1 - 2/r) / r²
Γ^t_tr = 1 / (r (r - 2))
Γ^r_rr = -Γ^t_tr
Γ^r_θθ = -(r - 2)
Γ^θ_rθ = 1/r
```

`Γ^r_tt` is also `-½ g^{rr} ∂_r g_tt`. At `r = 10` the closed form is
`0.008` and a central difference with `h = 1e-6` is `7.999999973585e-3`.

Code: `include/blackhole_ds/metrics/schwarzschild_christoffel.hpp`.

### Already in the tree, not re-derived here

These were checked by the existing tests. This list is only so the file is
complete. Limits are the ones in those headers.

| Piece | Tier | Source named in the header | Test |
|---|---|---|---|
| Schwarzschild radii `r_s`, `3M`, `6M` | analytic | Schwarzschild 1916; MTW | `smoke_tests` |
| Weak-field deflection `4M/b` | numerical integral of an analytic equation | Eddington 1919 | `geodesic_tests` |
| Kerr null geodesic, Carter `Q`, Mino-time `R(r)` and `Θ(θ)` | equations analytic; RK4 numerical | Carter 1968; Bardeen 1973; Chandrasekhar 1983 | `kerr_geodesic_tests` (drift of `Q` and of the on-shell residuals) |
| Kerr shadow curve | analytic | Bardeen 1973 | `kerr_shadow_tests`, including `a → 0` back to `sqrt(27)` |
| Disk redshift `g` | analytic | Cunningham 1975 | `disk_tests`, `kerr_disk_tests` |
| Hawking temperature | the formula is the standard semiclassical expression; the project labels Hawking radiation as speculative in the source cards | Hawking 1974 | `thermo_tests` |
| Quasinormal frequencies | numerical | Leaver; WKB; Berti–Cardoso–Will fit | `qnm_tests` |

The Kerr geodesic comment describes Mino time. The disk ray tracer does not
use that second-order integrator over the long observer-to-hole range; it
integrates the first-order on-shell system. Both are described in
`kerr_geodesic.hpp`.

## Certificates

Ran by CTest, with no extra tools:

- The integer `static_assert`s in `tests/classical_physics_tests.cpp`.
  They are the cleared-denominator forms of the photon-sphere equilibrium,
  the critical potential `1/27`, `b² = 27`, `E² = 8/9`, `L² = 12`, and the
  `a = 0` reduction of the BPT ISCO.
- The floating-point comparisons in the same file, with the numbers in the
  tables above.
- The older shadow, geodesic, Kerr, and disk tests.

Optional, not run in this environment and not required by CTest:

- `proofs/ClassicalGR.lean`. Same integer identities, core Lean 4, `rfl`,
  no Mathlib. `lean` was not installed on the machine that added the file.
  Run it with `lean proofs/ClassicalGR.lean` where Lean 4 is available.
  A failure there would mean the Lean file and the `static_assert`s have
  diverged; the C++ asserts are the ones CI executes.

## Known limits

- The picture of the disk is not the Page–Thorne flux.
- Kerr flux is a numerical integral. It is checked by convergence in the
  panel count and by the `a → 0` reduction. It is not an independent
  published table of `F(r, a)`.
- No covariant transfer. `I_obs = g⁴ F/π` is the vacuum invariant times
  Lambert's law.
- No Kerr escape angle. A ray that leaves the Kerr disk tracer is still
  recorded as background with no celestial direction.
- No Gralla–Lupsasca elliptic-integral geodesic. The cross-check against
  that closed form is not done. The RK Carter integrator is the one that
  exists.
- Eshkol did not differentiate anything in this run. Moonlab, QGTL, and
  Rosette Wire are not linked.
