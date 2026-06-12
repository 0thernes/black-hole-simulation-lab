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

## ADR-0005: External integrations are opt-in, ADR-gated, and adapter-only

- Date: 2026-05-26
- Status: accepted
- Context: Compounding on existing scientific-computing libraries is a core
  mission pillar (see `docs/vision/MISSION.md`). The relevant ecosystem is
  the tsotchke family of repositories: Eshkol, libirrep,
  quantum_geometric_tensor, PINN, quantum_rng, moonlab, etc. Adopting any
  of them naively is risky: license drift, supply-chain trust, build-time
  bloat, and abstraction leakage are all real costs.
- Decision: Every external integration follows the same policy:
  1. Documented under `external/<vendor>/<name>/README.md` with the
     upstream URL, pinned commit SHA, license, and adapter notes.
  2. Pulled via CMake `FetchContent` or git submodule. No upstream code
     committed into our tree.
  3. Pinned by commit SHA. Tracking a branch is a review-blocker.
  4. Gated behind a CMake option `BHDS_ENABLE_<NAME>` that defaults to
     OFF. The kernel must build and pass tests with every integration
     disabled.
  5. Wrapped by an adapter under
     `include/blackhole_ds/external/<name>.hpp`. The kernel never
     includes upstream headers directly.
  6. Documented in an ADR before merging, including scope, blast radius,
     license review, and rollback plan.
- Consequences:
  - Predictable build times for the default configuration.
  - Easy to roll back a problematic integration without touching the
     kernel.
  - Requires more discipline up front; the kernel cannot casually depend
     on an upstream feature.
- Alternatives considered:
  - Vendor upstream code into the repo (rejected: license risk, history
    bloat, drift).
  - Use a package manager like Conan or vcpkg (deferred: adds another
    moving part. Revisit when there are >= 3 active external integrations).

## ADR-0006: The project license is AGPL-3.0 (corrects ADR-0004)

- Date: 2026-06-12
- Status: accepted
- Context: The 2026-06-12 full repository audit (finding F-011) showed that
  the root `LICENSE` file has been the GNU Affero General Public License
  v3.0 since the baseline commit, while ADR-0004 and three docs claimed the
  project was MIT. ADR-0004 was written from a chat-history description of
  the repo instead of reading the actual file - a process failure worth
  recording: decisions must be made against on-disk reality.
- Decision: The project license is AGPL-3.0, matching the LICENSE file
  that has been in force since the baseline and matching the original
  intent of the AGPLv3 upload. All documentation that claimed MIT is
  corrected. Strong copyleft fits this project: it is an open research
  lab, and AGPL ensures derived simulation services share their changes.
- Consequences:
  - Contributors and downstream users operate under AGPL-3.0 terms.
  - External integrations must be license-compatible with AGPL-3.0
    (permissively-licensed upstreams like MIT/BSD/Apache-2.0 remain fine
    to depend on; the combined work is AGPL).
  - Any future relicensing requires a new ADR and consent of all
    copyright holders at that time.
- Alternatives considered:
  - Switch the LICENSE file to MIT to make ADR-0004 true retroactively
    (rejected: the AGPL file was the deliberate original upload, and
    weakening copyleft silently after publication is poor governance).

## ADR-0004: License remains MIT for now (SUPERSEDED by ADR-0006)

- Date: 2026-05-26
- Status: superseded by ADR-0006
- Note: This ADR recorded a decision about a LICENSE file that did not
  exist as described. The root LICENSE was already AGPL-3.0 when this was
  written. Kept for the historical record; see ADR-0006 for the correction
  and the active decision.
- Original (incorrect) decision text: keep the root LICENSE as MIT and
  defer the license question.
