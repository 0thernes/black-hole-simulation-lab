# Tsotchke Ecosystem Integration Map

`tsotchke` is a GitHub user who has published an unusually relevant set of
open-source scientific computing libraries in C and C++. Several of these
repositories map directly to modules we need for BlackHoleDS. This document
maps each tsotchke repository to its integration point and to the iteration
in which it should land.

This is a planning document, not an implementation. Adding any of these as
real dependencies requires an Architecture Decision Record (ADR) in
`docs/log/DECISIONS.md` and an entry in `docs/integrations/`.

## Why compound on this ecosystem

The Scientific Integrity Charter, Vision, and Mission all point to the
same rule: do not reinvent in isolation. Where credible open implementations
exist, we wrap or integrate them and pay for the abstraction with clarity,
not code volume.

`tsotchke` ships code that already does several of the heavy mechanical
tasks our roadmap requires: scientific-computing DSL, tensor kernels,
representation-theory primitives, physics-informed neural networks,
quantum random number generation, and a quantum-simulator backbone. Each
is independently usable from a small C or C++ adapter layer.

## Repository roster (verified 2026-05-26)

| Repo | Language | Stars | Role in BlackHoleDS |
|---|---|---|---|
| [tsotchke/eshkol](https://github.com/tsotchke/eshkol) | C++ (LISP-like DSL) | 115 | DSL layer for scientific kernels. Future home of high-level metric definitions and physics laws. |
| [tsotchke/quantum_geometric_tensor](https://github.com/tsotchke/quantum_geometric_tensor) | C | 452 | Hybrid classical-quantum tensor kernels. Candidate backend for tensor-heavy GR computations (Christoffel, Riemann, Ricci). |
| [tsotchke/spin_based_neural_network](https://github.com/tsotchke/spin_based_neural_network) | C | 85 | Spin-system simulation. Useful for analogies in the `speculative_extension` quantum modules. |
| [tsotchke/libirrep](https://github.com/tsotchke/libirrep) | C | 10 | SO(3), SU(2), O(3), SE(3) representation theory. Directly applicable to Kerr/Schwarzschild symmetry structures and spinor formalism. |
| [tsotchke/PINN](https://github.com/tsotchke/PINN) | C | 38 | Physics-Informed Neural Networks. Future option for surrogate models of expensive integrators. |
| [tsotchke/quantum_rng](https://github.com/tsotchke/quantum_rng) | C | 71 | Semi-classical quantum RNG. Reproducibility-friendly randomness for stochastic harnesses. |
| [tsotchke/moonlab](https://github.com/tsotchke/moonlab) | C | 24 | High-performance quantum computing simulator with CUDA/Metal. Reference architecture for our future GPU integration. |
| [tsotchke/tensorcore](https://github.com/tsotchke/tensorcore) | C / Metal | 2 | Apple-silicon tensor-core kernels. Reference; less applicable to RTX 5070 Ti hardware. |
| [tsotchke/PIMC](https://github.com/tsotchke/PIMC) | Java | 10 | Path Integral Monte Carlo. Pedagogical reference, not an integration target. |
| [tsotchke/simple_mnist](https://github.com/tsotchke/simple_mnist) | C | 108 | Pedagogical reference for hand-rolled neural net code. Not relevant directly. |
| [tsotchke/llm-arbitrator](https://github.com/tsotchke/llm-arbitrator) | TS | 24 | Orchestration scaffolding for multi-model agent workflows. Out of scope for the C++ kernel. |
| [tsotchke/homebrew-eshkol](https://github.com/tsotchke/homebrew-eshkol) | Ruby | 3 | Homebrew tap. Not applicable on Windows. |

## Integration sequencing

Integrations are sized by ADR-required scope, blast radius, and how much
they unlock for downstream iterations.

### Tier A: Prove the integration pattern (Iteration 6 of bootstrap)

These are documentation-only in this iteration. The goal is to define the
`external/` adapter pattern, the CMake `FetchContent` wiring, and the ADR
template, then merge with **no** real fetches.

- **External adapter pattern** under `external/<vendor>/<name>/`. Each
  directory holds a `README.md` describing the upstream commit pinned, the
  license, the wrapper module under `include/blackhole_ds/external/`, and
  the integration tests under `tests/external/`.
- **CMake FetchContent wiring** committed but commented out. Toggled per
  integration via `BHDS_ENABLE_<NAME>` options.

### Tier B: First real integration (target Q2)

- **`tsotchke/libirrep`**. Smallest blast radius, highest scientific
  relevance early. SO(3) and SU(2) representations show up immediately in
  spinor formulations and ringdown/quasi-normal-mode work. Adapter under
  `external/tsotchke/libirrep/` and `include/blackhole_ds/external/irrep.hpp`.
  Driven by an ADR describing the commit pin and the wrap surface.

### Tier C: Second wave (target Q3)

- **`tsotchke/eshkol`** as a DSL layer for metric definitions. Eshkol
  source files live under `eshkol/` with a build-time transpilation step
  producing C++ headers consumed by the kernel.
- **`tsotchke/quantum_geometric_tensor`** as an optional tensor backend.
  Wired behind a `BHDS_TENSOR_BACKEND` cmake option (default: builtin).

### Tier D: Speculative or hardware-dependent (target Year 2+)

- **`tsotchke/moonlab`** for GPU-accelerated portions, behind
  `BHDS_ENABLE_GPU`. Targets CUDA on the RTX 5070 Ti.
- **`tsotchke/PINN`** as a surrogate model for expensive integrator runs.
- **`tsotchke/quantum_rng`** for reproducible random streams in
  ensemble harnesses.

## Out of scope

- `tsotchke/llm-arbitrator`, `tsotchke/simple_mnist`,
  `tsotchke/homebrew-eshkol`, `tsotchke/tensorcore`, and `tsotchke/PIMC`
  are out of scope for the C++ kernel at this time.

## License compatibility

The project license is AGPL-3.0 (see [ADR-0006](../log/DECISIONS.md)).
Before any tsotchke integration lands, the upstream license must be
reviewed for AGPL-3.0 compatibility and recorded under `docs/legal/`.
Permissive upstream licenses (MIT/BSD/Apache-2.0) are compatible as
dependencies; the combined work remains AGPL. Each integration ADR must
include the upstream license and any compatibility caveats.
