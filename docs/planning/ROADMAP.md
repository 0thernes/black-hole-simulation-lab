# Roadmap

The destination is a **visual, GPU-accelerated black hole simulation** that
runs on RTX-class consumer hardware and is scientifically honest about
every pixel. This roadmap is the ordered path from the current validated
analytic core to that destination.

Each milestone has an explicit **exit criterion**. A milestone is not
"done" until its exit criterion is met, tested, and merged.

```mermaid
flowchart LR
    M0[M0 Analytic core<br/>DONE] --> M1[M1 Geodesic integrator]
    M1 --> M2[M2 Data contract<br/>+ truth tiers]
    M2 --> M3[M3 CPU lensing]
    M3 --> M4[M4 GPU ray marcher<br/>VISUAL PROTOTYPE]
    M4 --> M5[M5 Kerr + accretion]
    M5 --> M6[M6 Ecosystem + scale]
```

---

## M0 — Analytic core and research infrastructure (DONE)

- Strong-typed units, exact Schwarzschild/Kerr observables, CLI, CSV export.
- RK4 + adaptive Dormand-Prince integrators with convergence tests.
- Brain corpus, source-card corpus, vision/charter, ADRs, CI green.

**Exit criterion (met):** every analytic value matches the literature to
the documented tolerance; CI is green on `windows-latest`; all gates gate.

## M1 — Geodesic integrator (IN PROGRESS)

Build the Schwarzschild null-geodesic right-hand side and integrate it.

Done (2026-06-13):
- Equatorial photon orbit equation `d^2u/dphi^2 + u = 3u^2` integrated by
  RK4 (`include/blackhole_ds/geodesics/schwarzschild_photon.hpp`).
- Critical impact parameter `b_crit = sqrt(27) M`, escape/capture/critical
  classification, photon turning point, total light-deflection angle.
- Validated against the weak-field Eddington deflection `4M/b` and the
  photon-sphere capture boundary (`tests/geodesic_tests.cpp`).
- `--deflection <b/M>` CLI flag for runnable, labeled output.

Remaining for M1:
- A full equatorial geodesic in (t, r, phi) with conserved E, L tracked by
  the Kahan accumulator (the current form integrates the orbit shape u(phi);
  the next step also tracks affine/coordinate evolution for a ray tracer).
- Tighten the deflection regression to a published high-precision table.

**Exit criterion:** a ray bundle reproduces the analytic deflection curve
and the photon-sphere capture boundary within tight tolerance, with a
regression test. (Partially met: the deflection and capture boundary are
validated; the conserved-quantity-tracked ray remains.)

## M2 — Data contract and truth tiers

- Add the `model_status` column to `runs`; update the C++ exporter and the
  Python harness together (schema-change ADR).
- JSON exporter alongside CSV.
- First C++ → SQLite path.
- E2E test: run → emit → ingest → validate.

**Exit criterion:** a reproducible run produces queryable, truth-tier
labeled data that round-trips through the schema, verified by an E2E test.

## M3 — CPU gravitational lensing

- One ray per image pixel; integrate backward from the camera.
- Map escaped rays to a background; map captured rays to the shadow; map
  rays that cross the disk plane to accretion-disk emission.
- Schwarzschild first (no spin), equatorial thin disk.

**Exit criterion:** a CPU render of the Schwarzschild shadow + lensed disk
whose shadow diameter matches 2√27 M (the analytic value) to within one
pixel, with the image checked into `docs/` as a reference.

## M4 — GPU ray marcher (the visual prototype)

- Port the per-pixel ray integration to CUDA (one thread per pixel).
- Target the RTX 5070 Ti (~8900 CUDA cores, 12 GB GDDR7).
- Interactive or near-interactive frame times for a moderate resolution.
- Honest tier labeling baked into the render metadata (geometry tiers
  analytic/numerical; color mapping `visualization_metaphor`).

**Exit criterion:** the GPU render matches the M3 CPU reference image to
within numerical tolerance, runs at least an order of magnitude faster, and
ships with a documented build path for RTX-class hardware.

## M5 — Kerr, spin, and accretion physics

- Kerr null geodesics (Carter constant).
- Frame dragging and the asymmetric shadow.
- Doppler beaming and gravitational redshift on the disk
  (`numerical_approximation` / `observational_constraint` as appropriate).

**Exit criterion:** a Kerr render shows the characteristic asymmetric
photon ring; shadow asymmetry vs spin matches published EHT-style curves
qualitatively, with the comparison documented.

## M6 — Ecosystem integration and scale

- First `tsotchke/libirrep` integration (symmetry math) per ADR-0005.
- Eshkol DSL layer for metric definitions (ADR + staged rollout).
- A/B integrator harness, PI step controller, performance regression suite.
- Optional: GRMHD / numerical-relativity comparison against the Einstein
  Toolkit or BHAC as a benchmark.

**Exit criterion:** at least one external integration is wired,
SHA-pinned, and opt-in per the policy; performance regressions are caught
by CI.

---

## Cross-cutting tracks (run continuously)

- **Audit remediation.** Burn down the 47 open findings from the
  2026-06-12 report; never let the open-critical count rise above zero.
- **Research corpus.** Grow the source cards and brain profiles as physics
  is implemented; every formula gets a source card before it ships.
- **CI/CD.** Keep the matrix green; add checks as the surface grows.

## Sequencing rule

Physics never outruns evidence, and code never outruns tests. A milestone
that would add a visual or a number without a validation path is reordered
until the validation exists. Beauty is the reward for correctness, not a
substitute for it.
