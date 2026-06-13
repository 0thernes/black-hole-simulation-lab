# Time and Space Complexity Reference

This document records the asymptotic complexity of every non-trivial
algorithm in the repository, current and planned. It is part of the
engineering contract: a change that worsens a documented complexity class
without justification is a review-blocker.

Notation: `N` = state dimension (geodesics: 8), `M` = number of integration
steps, `P` = number of pixels in a render, `D` = number of corpus entries,
`L` = lines/characters in a file. All bounds are worst-case unless noted.

---

## 1. C++ numerical kernel

### 1.1 Physical units (`include/blackhole_ds/units.hpp`)

| Operation | Time | Space | Notes |
|---|---|---|---|
| `Quantity` arithmetic (`+ - * /`, comparisons) | O(1) | O(1) | All `constexpr`, zero runtime cost after inlining. |
| `KahanAccumulator::add` | O(1) | O(1) | Compensated summation; one extra FLOP vs naive add to bound round-off to O(epsilon) instead of O(M epsilon). |
| `validators::*` | O(1) | O(1) | Pure arithmetic, `constexpr`, MSVC-safe (no std-math). |

The Kahan accumulator is the one place we trade a constant factor of
arithmetic for a dramatically better *error* growth: naive summation of M
terms accumulates O(M * epsilon) round-off; Kahan keeps it O(epsilon),
independent of M. Critical for conserved quantities over long integrations.

### 1.2 Metrics (`include/blackhole_ds/metrics/`)

| Function | Time | Space | Notes |
|---|---|---|---|
| `schwarzschild::radius_m / photon_sphere_m / isco_m` | O(1) | O(1) | Closed form, one multiply. |
| `kerr::isco_dimensionless` | O(1) | O(1) | Bardeen-Press-Teukolsky closed form: a handful of `cbrt`/`sqrt`. |
| `kerr::photon_sphere_dimensionless` | O(1) | O(1) | One `acos` + one `cos`. |

No metric evaluation iterates. Every observable is closed-form.

### 1.3 ODE state ops (`include/blackhole_ds/integrators/ode_state.hpp`)

| Operation | Time | Space | Notes |
|---|---|---|---|
| `add`, `axpy`, `scale` | O(N) | O(N) | N fixed and small (stack `std::array`, no heap). |
| `weighted_rms_norm` | O(N) | O(1) | One pass; one `sqrt` on the cold path. |

### 1.4 RK4 fixed-step integrator (`include/blackhole_ds/integrators/rk4.hpp`)

| Operation | Time | Space | Notes |
|---|---|---|---|
| `rk4_step` | 4 derivative evals + O(N) | O(N) | Global error O(h^4). |
| `rk4_integrate` (M steps) | O(M) derivative evals | O(N) | No allocation in the loop; total error O(h^4) = O((1/M)^4). |

RK4 is order-4: halving the step size cuts the error by ~16x (verified in
`tests/integrator_tests.cpp`, which checks the empirical ratio lands in
[12, 20]).

### 1.5 Adaptive Dormand-Prince 5(4) (`include/blackhole_ds/integrators/rk45.hpp`)

| Operation | Time | Space | Notes |
|---|---|---|---|
| `dp_step` | 6 derivative evals (FSAL) + O(N) | O(N) | 5th-order solution + 4th-order embedded error estimate. |
| `rk45_integrate` | O(steps) evals; steps depend on tolerance and stiffness | O(N) | Step count is *adaptive*, not fixed: large where smooth, small near features. |

The key complexity property of the adaptive method is that it spends work
*where the solution needs it*. For a smooth problem it takes few large
steps; near the photon sphere, where null geodesics are exponentially
sensitive (positive Lyapunov exponent), it automatically shrinks the step.
The step-size controller is the elementary I-controller
`h_new = h * safety * e^(-1/5)` clamped to `[min_scale, max_scale]`, where
`e` is the tolerance-normalized error norm (target 1.0). FSAL reuse means
an accepted step costs 6 derivative evaluations, not 7.

Planned upgrade: a PI controller (`e_n^(-a) * e_{n-1}^(b)`) reduces step
rejections on stiff segments. Tracked in the roadmap.

### 1.6 CSV writer (`include/blackhole_ds/data/csv_writer.hpp`)

| Operation | Time | Space | Notes |
|---|---|---|---|
| `write_row` | O(1) amortized | O(1) | Streams at `max_digits10` (17 sig figs); saves/restores stream flags. |
| Full export of R rows | O(R) | O(1) | Streaming; no buffering of the whole table. |

### 1.7 CLI (`src/cli/main.cpp`)

| Operation | Time | Space | Notes |
|---|---|---|---|
| Argument parse | O(A) in arg count A | O(1) | Single pass. |
| `emit_text` / `emit_csv` over S spin steps | O(S) | O(1) | One closed-form metric eval per row. |

---

## 2. Python tooling

### 2.1 Corpus generators (`scripts/brains/build_brains.py`, `scripts/research/build_source_cards.py`)

| Operation | Time | Space | Notes |
|---|---|---|---|
| Generate D profiles/cards | O(D * F) | O(D) | F = fields per entry; one pass, deterministic output. |
| Build INDEX / MANIFEST | O(D log D) | O(D) | The `log D` is the sort by slug/year. |

Deterministic: dates come from the seed JSON, so regeneration is a pure
function of the seed. The CI drift gate (`git diff --exit-code`) relies on
this O(D) idempotence.

### 2.2 Validators (`scripts/brains/validate_brains.py`, `scripts/research/validate_source_cards.py`)

| Operation | Time | Space | Notes |
|---|---|---|---|
| Structural checks over D entries | O(D * L) | O(D) | L = file size; one parse per file. |
| Orphan scan | O(D) | O(D) | Set membership on resolved paths. |
| XSD validation (lxml, when available) | O(D * L) | O(L) | One schema compile + D document validations. |
| JSON Schema validation (jsonschema) | O(D * S) | O(S) | S = schema size. |

### 2.3 Umbrella validator (`scripts/Validate-ResearchOS.py`)

| Operation | Time | Space | Notes |
|---|---|---|---|
| Required-file presence | O(R) | O(1) | R = required-file count (currently 57). |
| Tracked-file scans | O(T) | O(T) | T = tracked files; one `git ls-files` + regex per path. |
| Sub-validator dispatch | sum of 2.1-2.2 | - | Subprocess per corpus. |

### 2.4 Data-science harness (`tools/blackhole_ds_harness.py`)

| Operation | Time | Space | Notes |
|---|---|---|---|
| Generate an ensemble of n runs | O(n) | O(n) | One closed-form physics eval per run. |
| SQLite export | O(n) inserts | O(1) streaming | One transaction. |
| xlsx export (openpyxl) | O(n) | O(n) | Holds the workbook in memory. |

---

## 3. Planned algorithms (not yet implemented)

These are recorded now so their target complexity is a design constraint,
not an afterthought.

| Algorithm | Target time | Target space | Notes |
|---|---|---|---|
| Null-geodesic integrator (Schwarzschild) | O(M) per ray, M adaptive | O(N) | Reuses `rk45_integrate` with N=8; M depends on impact parameter (blows up near the photon sphere). |
| Photon-ring image (CPU) | O(P * M_avg) | O(P) | P pixels, each a ray; M_avg the mean steps to escape/capture. Embarrassingly parallel over pixels. |
| Photon-ring image (GPU/CUDA) | O(P * M_avg / cores) wall-clock | O(P) device | One thread per pixel; the RTX 5070 Ti's ~8900 CUDA cores give the constant-factor win. |
| Adaptive Mesh Refinement (future GRMHD) | O(active cells), sublinear in resolution | O(active cells) | Refine only where gradients are large. |
| k-d tree / BVH for accretion-disk intersection | O(log K) query, O(K log K) build | O(K) | K disk elements; avoids O(K) brute-force ray-disk tests. |

The headline scaling story for the visual goal: **the render is
embarrassingly parallel over pixels**, which is exactly why a GPU is the
right tool. Each pixel's ray is independent, so wall-clock time drops by
the core count. The per-ray work is `O(M)` adaptive steps, and the
adaptive integrator keeps `M` small except for the measure-zero set of
rays that asymptote to the photon sphere.

---

## 4. Complexity review checklist

When adding or changing an algorithm:

1. State the time and space complexity in the header/docstring.
2. Add a row to the relevant table above.
3. If a loop is introduced, justify why its bound is acceptable.
4. If you replace an O(f) algorithm with O(g), say why (and add a
   benchmark if the constant factors matter).
5. Numerical methods must also state their *error* order, not just their
   step complexity. Step count and accuracy are different axes.
