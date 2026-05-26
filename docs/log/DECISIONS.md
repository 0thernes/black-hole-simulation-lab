# Architecture Decision Records (ADRs)

This file records meaningful, hard-to-reverse architecture and design
decisions for the project. Cheap, easy-to-reverse choices belong in commit
messages or in `DAILY_LOG.md`.

Each ADR is short and structured:

```text
## ADR-NNNN: <short title>

- Date: YYYY-MM-DD
- Status: proposed | accepted | superseded by ADR-XXXX | rejected
- Context: the problem and constraints that forced a decision.
- Decision: what we are doing.
- Consequences: what becomes easier, harder, or off-limits because of this.
- Alternatives considered: the runner-up options and why they lost.
```

Numbers are assigned in order. Do not renumber.

---

## ADR-0001: C++20 as the primary implementation language

- Date: 2026-05-26
- Status: accepted
- Context: The project needs a high-performance language for general-relativity
  numerics, geodesic integration, and eventually GRMHD-class work. The author
  has GPU-capable hardware (RTX 5070 Ti 12 GB GDDR7) and wants to interop with
  the tsotchke ecosystem, which is C and C++ heavy.
- Decision: Use C++20 as the primary implementation language. Use the strong
  types in `include/blackhole_ds/units.hpp` everywhere physical quantities
  appear. Disable compiler extensions (`CMAKE_CXX_EXTENSIONS OFF`).
- Consequences:
  - Easier interop with `tsotchke/eshkol` (LISP-like for scientific computing
    in C++), `quantum_geometric_tensor`, and `spin_based_neural_network`.
  - Higher build-system complexity than a pure-Python or pure-Eshkol project.
  - Pays compile-time cost in exchange for type safety on physical units.
- Alternatives considered:
  - Pure Python prototype: faster to write, too slow to scale to integrators.
  - Eshkol as primary: powerful but author is in learning phase; safer to use
    Eshkol as a DSL layer once the C++ core stabilizes.
  - C: fewer abstractions but no concepts/templates for typed units.

## ADR-0002: Scientific truth labels are required on every published value

- Date: 2026-05-26
- Status: accepted
- Context: It is easy to mix exact analytic GR results with numerical
  approximations, observational data, visual metaphors, and speculative
  ideas. That mix is how science codebases lie by accident.
- Decision: Every value, plot, table, exported row, and visualization must
  declare one model status from the list defined in
  `docs/architecture/ARCHITECTURE.md`:
  - `analytic_classical`
  - `numerical_approximation`
  - `observational_constraint`
  - `visualization_metaphor`
  - `pedagogical_simplification`
  - `speculative_extension`
- Consequences:
  - Frontier ideas like Hawking radiation, Page curves, fuzzballs, and
    AdS/CFT can live in the repo without contaminating analytic outputs.
  - Code reviews must include a "label correctness" check.
- Alternatives considered:
  - Single "scientific" flag (rejected: erases real epistemic distinctions).
  - Free-form notes per value (rejected: too easy to drift).

## ADR-0003: Local Git working tree is the source of truth

- Date: 2026-05-26
- Status: accepted
- Context: Earlier in the project, ChatGPT generated ZIPs and patches that
  described large repo states. None of them were applied to the local
  working tree. The repo and the descriptions diverged.
- Decision: The local Git working tree at
  `Z:\Orca\Workspaces\Stress Test Agents Maxxxing` and its remote
  `origin = https://github.com/0thernes/Stress-Test-Agents-Maxxxing.git` are
  the authoritative source of truth. ZIPs and patches are intake artifacts,
  archived under `_incoming/` (ignored), applied with a documented script,
  and committed before being trusted.
- Consequences:
  - All AI-generated work must land as files in the repo or it does not
    exist.
  - The daily workflow is: edit, validate, build, test, commit, push.

## ADR-0004: License remains MIT for now; AGPLv3 upload kept as a reference

- Date: 2026-05-26
- Status: accepted
- Context: An uploaded `LICENSE` file from a prior session contained AGPLv3,
  while the repo's existing LICENSE is MIT. Switching licenses is a major
  governance change that affects every contributor and downstream user.
- Decision: Keep the root `LICENSE` as MIT. Preserve the AGPLv3 upload as a
  reference under `docs/reference/user-supplied/` if it is added later, with
  a clear note that it is not the active license. Defer the license question
  until the project has a meaningful contributor base or a real use case
  that depends on copyleft semantics.
- Consequences:
  - Anyone can use the code under MIT terms today.
  - A future license change would require an explicit ADR, contributor
    sign-off, and updates to all source headers.
