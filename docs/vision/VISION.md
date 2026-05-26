# Vision

## What this project is

BlackHoleDS / OPPENGROK is a research-first, type-safe, scientifically
honest black hole simulation and data science lab written primarily in
C++20. The long-term aim is a simulator that I can run locally on consumer
hardware (Intel Core Ultra 9 275HX, NVIDIA RTX 5070 Ti 12 GB GDDR7, 80 GB
DDR5) to study black holes, related general-relativity phenomena, and the
quantum/cosmological frontier in a way that is reproducible, auditable, and
honest about what it is and is not modeling.

## Why it exists

I am in a long-term physics and mathematics learning phase. Choosing one
massive, structured topic and building a real codebase around it is how I
intend to internalize the field. Black holes sit at the intersection of
classical general relativity, differential geometry, thermodynamics,
information theory, quantum field theory in curved spacetime, gravitational
wave astronomy, and modern observational astrophysics. They are also the
closest thing physics has to a stress test for almost every framework we
have.

A real codebase is a forcing function. It does not let you wave at an
equation; it makes you write the units, the integrator, the test, and the
plot.

## What it is not

It is not an attempt to replace established research tools like the
Einstein Toolkit, GRChombo, BHAC, iharm3D, or the Black Hole Perturbation
Toolkit. Those exist, they are excellent, and we should treat them as
benchmarks and integration targets, not competitors.

It is not a venue for hype. Hawking radiation, Page curves, fuzzballs,
islands, holography, AdS/CFT, primordial black holes, and any frontier
idea are welcome as `speculative_extension` modules, clearly separated
from validated classical results.

It is not a hobby script. The bar is professional engineering:
type-checked physical units, reproducible builds, tests that grow with
risk, daily commits, and an auditable history. The fact that the author is
in a learning phase makes the discipline more important, not less.

## Three-Year Horizon

- **Year 1:** Strong analytic foundation. Schwarzschild and Kerr observables
  in C++, validated against literature values. A first geodesic integrator
  with documented error control. Real data exports to CSV/JSON/SQLite.
  Source-card corpus and brain/soul reasoning lenses populated. First
  integrations with the `tsotchke` ecosystem.
- **Year 2:** Numerical relativity comparisons. Photon-ring renderer.
  Ray-traced visualization that consumes simulation output. Gravitational
  wave catalog integration. First A/B integrator harness. First publishable
  technical report (even if internal).
- **Year 3:** Cross-validation with at least one established simulator
  (Einstein Toolkit or BHAC). A documented Eshkol DSL layer for scientific
  kernels. Real, reproducible end-to-end runs that take advantage of the
  RTX 5070 Ti for radiative transfer or perturbation-theory sweeps.

The three-year horizon is a direction, not a contract.
