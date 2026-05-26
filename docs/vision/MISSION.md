# Mission

## Current Mission (12-month horizon)

Build a scientifically auditable, type-safe, locally runnable C++ black
hole simulation lab that I can use to learn and verify general relativity
results, then ship the first reproducible end-to-end run that produces
exportable data, validated tests, and a documented visualization.

## Six Mission Pillars

1. **Type safety on physical quantities.** The `units.hpp` strong-typed
   `Length`, `Time`, `Mass`, `Energy`, `AngularMomentum`, `Velocity`, and
   `Dimensionless` types are non-negotiable. Adding meters to seconds must
   be a compile error.
2. **Scientific honesty.** Every value, plot, table, and exported row is
   labeled with one of the six `model_status` truth tiers. Frontier ideas
   are welcome but never disguised as validated results.
3. **Reproducibility.** Every run has a seed, a parameter set, a model
   status, and a git commit. Two runs with the same inputs produce
   bitwise-identical outputs (or documented numerical tolerances).
4. **Daily forward motion.** Daily commits, weekly milestones, monthly
   reviews. Small, focused commits with meaningful messages beat
   end-of-month megabranches.
5. **Tier-1 hygiene.** The repo respects the Gold Standard SOP discipline:
   weighted reviews, anti-pattern registries, secret-hygiene floors, and
   CI gates. We measure quality, not perform it.
6. **Compounding on the right shoulders.** Build on the `tsotchke`
   ecosystem (Eshkol for DSL, quantum_geometric_tensor for tensor kernels,
   libirrep for spacetime symmetry math, PINN for physics-informed ML, and
   so on). Build on Einstein Toolkit, BHAC, iharm3D, and Black Hole
   Perturbation Toolkit as reference benchmarks. Never reinvent in
   isolation.

## Quarterly Objectives

### Q1 (current quarter)

- Land Iterations 1 through 8 of the bootstrap plan: foundation sanity,
  repo skeleton, vision/ERM, brain/soul XML, source cards, tsotchke
  integration plan, C++ modularization, and daily workflow automation.
- Split `BlackHoleDS.cpp` into core/metrics/integrators/data modules.
- Add a CLI with mass, spin, and output-format flags.
- Add CSV and JSON exporters that round-trip with `data/schema.sql`.
- Populate the first 90 brain XML profiles (30 each: mathematicians,
  physicists, astronomers) and the first 20 source cards.

### Q2

- Implement a first numerical geodesic integrator with documented error
  bounds.
- Add A/B harness for integrator comparison.
- Wire the first `tsotchke` integration (likely `libirrep`).
- Extend brain/soul corpus to 300 profiles.
- Extend source cards to 60.

### Q3

- Add a photon-ring computation pipeline (analytic and numerical).
- Add first visualization prototype that consumes simulation state via the
  data contract.
- Cross-validate ISCO and photon-sphere values against literature
  to within 1e-9 in geometric units.

### Q4

- Integrate with at least one open-data source: EHT public products,
  GWOSC, or HEASARC.
- Publish a first internal technical report (PDF/Markdown) summarizing
  what the lab can do, what it cannot, and why.

## Definition of Success for Year 1

- A user with the same hardware can clone, build, and run the lab in under
  five minutes and get scientifically meaningful, labeled output.
- Every analytic value in the output matches a peer-reviewed reference to
  within stated tolerance.
- Every dependency, including the `tsotchke` integrations, is pinned and
  reproducible.
- The repo is something I would not be embarrassed to show a professional
  numerical-relativity researcher.
