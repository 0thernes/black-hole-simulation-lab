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

## ADR-0007: clang-format is canonical and enforced in CI

- Date: 2026-06-12
- Status: accepted
- Context: The C++ was hand-formatted with no automated enforcement. For a
  repo aiming to pass professional scrutiny, "the author's eye" is not a
  defensible formatting policy: it invites style drift and bikeshedding in
  review. A formatter that is enforced is worth more to a reviewer than
  manual alignment that is occasionally prettier.
- Decision:
  1. Adopt `.clang-format` (LLVM base, 4-space indent, 80-column target,
     `PenaltyBreakString` high to avoid splitting most string literals) as
     the single source of truth for C++ style.
  2. Reformat the entire C++ tree once to conform; the result is
     idempotent under the config.
  3. Enforce it in CI with a `clang-format --dry-run --Werror` job, using
     `clang-format==19.1.7` pinned via pip so the CI version exactly
     matches the version the tree was formatted with (cross-version reflow
     differences cannot cause spurious failures).
  4. Provide `scripts/dev/format.ps1` so contributors apply or check
     formatting locally before pushing.
- Consequences:
  - Style is no longer a review topic; the bot decides.
  - A small cosmetic cost: long string literals in `<<` chains may be split
    into adjacent literals (compile-time concatenation, behavior
    identical).
  - Contributors must run the formatter or CI will reject the PR.
- Alternatives considered:
  - Advisory-only format check (rejected: a gate that does not gate is
    exactly the anti-pattern the 2026-06-12 audit flagged elsewhere).
  - `apt`-installed clang-format in CI (rejected: version drift vs the
    local formatting version causes spurious failures).

## ADR-0008: Explicit line-ending policy via .gitattributes

- Date: 2026-06-12
- Status: accepted
- Context: The repo had only `* text=auto`, which normalizes to LF in the
  repository but checks out CRLF on Windows. That produced constant "LF
  will be replaced by CRLF" warnings and, more seriously, risked CRLF
  contaminating generated shell artifacts (the pre-commit hook's
  `#!/bin/sh` shebang). Audit finding F-022.
- Decision: Add explicit `eol` attributes. Source, scripts, docs, data,
  and config that must be LF are pinned `eol=lf`; PowerShell is pinned
  `eol=crlf` (its native convention); binary asset types are marked
  `binary`. The pre-commit hook installer additionally normalizes its
  generated hook to LF in code, so it is robust regardless of how the
  `.ps1` itself is checked out.
- Consequences:
  - Deterministic line endings across platforms; no more churn warnings on
    the files that matter.
  - A one-time `git add --renormalize .` applies the policy to existing
    files.

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

## ADR-0009: Relicense to proprietary, All Rights Reserved (supersedes ADR-0006)

- Date: 2026-06-13
- Status: accepted
- Context: ADR-0006 set the license to AGPL-3.0 on the reasoning that an open
  research lab benefits from strong copyleft. On review, the author's intent
  for this specific body of work has changed: the simulation kernel (the
  compile-time dimensional-units system, the Carter-separated Mino-time Kerr
  geodesic engine, the closed-form Bardeen shadow renderer, the truth-tier
  discipline, and the brain/soul reasoning-lens corpus) is treated as novel,
  proprietary intellectual property. The author is the sole copyright holder
  (see `AUTHORS`; `git shortlog -sn` shows a single contributor), so a
  relicense requires no third-party consent. ADR-0006 itself anticipated this:
  "Any future relicensing requires a new ADR and consent of all copyright
  holders at that time" — this ADR is that record.
- Decision: The project license is **Proprietary — All Rights Reserved**. The
  root `LICENSE` is replaced with a proprietary license that reserves all
  rights, grants only a limited source-viewing permission, grants no patent
  rights, prohibits use/copying/modification/redistribution and use as ML
  training data absent a separate signed agreement, takes contributions by
  assignment (inbound != outbound), and disclaims warranty and liability. Every
  C/C++ source file's SPDX tag is changed from `AGPL-3.0-or-later` to
  `LicenseRef-Proprietary-AllRightsReserved`, and the copyright line gains
  "All Rights Reserved." `NOTICE`, `README`, `CONTRIBUTING`, `HIERARCHY`, the
  two integration docs, and the inspiration briefing are updated to match.
- Consequences:
  - No one may use, build, redistribute, or derive from the Work without a
    written license from the author. This is intentional for novel IP.
  - The dependency calculus inverts: permissive upstreams (MIT/BSD/Apache-2.0)
    may be linked in with notices preserved; **copyleft upstreams
    (GPL/AGPL/LGPL-static) may NOT** be incorporated without a separate
    commercial license, because they would force this Work open. ADR-0005's
    integration gate must enforce this.
  - Historical audit documents (`docs/audits/*`, `CHANGELOG` entries) that
    state "AGPL" remain unedited as a point-in-time record, consistent with
    ADR-0003 discipline. They describe the past, not the present.
  - The author may still open-source any explicitly designated future version
    (LICENSE §9); this decision does not foreclose that, it just makes the
    default closed.
- Alternatives considered:
  - Keep AGPL-3.0 (rejected: does not match the author's intent to hold this
    as novel proprietary IP; AGPL would compel disclosure of any networked
    derivative).
  - Dual-license (open core + commercial) (deferred: more governance than a
    solo research lab needs today; revisit if external demand appears).
  - A custom "source-available, non-commercial" license such as PolyForm
    Noncommercial or BUSL (considered: viable, but a plain All-Rights-Reserved
    proprietary grant is the least ambiguous expression of the author's intent
    right now and is trivially loosened later under §9).

## ADR-0010: Canonical working-tree path moves into the Vibe-Coded (AI) / CLAUDECODE workspace

- Date: 2026-06-13
- Status: accepted
- Context: ADR-0003 named `Z:\Orca\Workspaces\Stress Test Agents Maxxxing` as
  the authoritative local working tree. The author consolidates all
  Claude-Code-managed projects under `Z:\[Vibe Coded (AI)]\CLAUDECODE\` (siblings:
  "Chaos Entropy Art Engine", "Cosmogonic Quantum Mechalogodrom",
  "Professional Calculator", "Art Provenance Vault", etc.). Keeping this repo
  in a separate `Orca\Workspaces` tree fragments the portfolio.
- Decision: Move the entire working tree (including `.git`, excluding nothing)
  to `Z:\[Vibe Coded (AI)]\CLAUDECODE\Black Hole Simulation Lab`. The Git
  history, the `origin` remote
  (`https://github.com/0thernes/Stress-Test-Agents-Maxxxing.git`), and the
  GitHub repo identity are unchanged — only the local filesystem path moves.
  This ADR amends, but does not delete, the path stated in ADR-0003.
- Consequences:
  - All local automation that hard-codes the old path
    (`CONTRIBUTING.md` daily-workflow block, any `scripts/`/`.vscode/` paths)
    must be updated; the move is a one-time rename on the same Z: volume, so it
    is fast and the `.git` directory travels intact.
  - Prior Claude Code session history keyed by the old path does not follow the
    move; this is expected and harmless.
  - The GitHub remote and clone URL are unaffected; `git remote -v` still
    points at the same origin after the move.
- Alternatives considered:
  - Re-clone fresh at the new path (rejected: loses local uncommitted work and
    ignored build state; a move preserves everything).
  - Leave it in `Orca\Workspaces` (rejected: the author explicitly wants one
    consolidated CLAUDECODE portfolio).

## ADR-0011: Quasinormal-mode (QNM) module

- Date: 2026-06-14
- Status: accepted
- Context: Catalogue problem #11 (testing the no-hair theorem / black-hole
  spectroscopy) is the catalogue entry most coupled to this simulator, but the
  coupling was aspirational — the kernel rendered Kerr shadows yet had no
  ringdown/QNM capability, so the cross-link pointed at nothing concrete.
- Decision: Add a header-only QNM module under `include/blackhole_ds/qnm/`:
  `leaver.hpp` (Leaver continued-fraction solver — the Schwarzschild radial
  recurrence plus the Kerr coupled angular-spheroidal + radial recurrences, with
  n-continuation seeding for overtones and spin continuation for Kerr),
  `wkb.hpp` (an independent 3rd-order WKB cross-check), `kerr_fit.hpp`
  (the Berti-Cardoso-Will closed-form fit), and `spectroscopy.hpp` (physical
  ringdown frequencies/damping times and the mass-independent no-hair
  fingerprint, `no_hair_signature`). All outputs carry
  `TruthLabel::NumericalApproximation`. `tests/qnm_tests.cpp` validates against
  published Berti/Leaver/Konoplya reference values to < 1e-4, and the two
  independent solvers (Schwarzschild s=+2 Regge-Wheeler vs Kerr s=-2 Teukolsky)
  agree at a=0 — an internal oracle cross-check.
- Consequences:
  - The catalogue #11 `simulator_angle` now references real, verified code; the
    no-hair test has a computational home (`no_hair_signature(j)` returns the
    spin-only Kerr fingerprint a measured ringdown is checked against).
  - `std::complex<double>` enters the kernel for the first time here — justified,
    QNM frequencies are intrinsically complex; the public result structs still
    expose real/imag doubles in the project idiom.
  - New CI test target `blackhole_ds_qnm_tests` (13th test).
- Alternatives considered:
  - WKB-only (rejected: ~few-percent accuracy on the imaginary part for low l;
    a cross-check, not a precision tool — Leaver is exact to machine precision).
  - Pre-tabulated frequencies (rejected: not a solver; cannot explore (M, j)).
  - Runtime dependency on Leo Stein's `qnm` Python package (rejected: adds a
    language/runtime dependency; instead used only during development to
    cross-check the published oracle, not imported by the deliverable).

## ADR-0012: Authorship attribution and AI-assistance disclosure

- Date: 2026-06-14
- Status: accepted
- Context: The repository is public and the companion catalogue is being made
  public. Authorship and the role of AI in producing the work must be stated
  honestly, and the human author credited.
- Decision: The human author, director, and reviewer — the Human-in-the-Loop —
  is **Alexander Donahue** (GitHub `0thernes`, `0_0@0thernes.art`). The code and
  documentation were produced with substantial AI assistance (Anthropic Claude /
  Claude Code and a multi-agent research-and-verification system) under his
  direction and review. This is recorded in `CITATION.cff` and `AUTHORS`.
  Physics results are validated against independent published references (the
  oracle pattern), not asserted on authority.
- Consequences:
  - Clear, honest academic citation; AI involvement is disclosed rather than
    hidden.
  - The proprietary All-Rights-Reserved license (ADR-0009) is unchanged — public
    visibility is source-viewing per LICENSE §2, and scholarly citation and
    academic review are expressly welcomed (citation is fair use and needs no
    grant).
- Alternatives considered:
  - Omit the AI-assistance disclosure (rejected: dishonest).
  - Claim unaided sole human authorship (rejected: false).
