# Tier-1 Full Repository Audit & Relocation Review — 2026-06-13

> Scope: 100% architecture, source, docs, build, and test review of the
> OPPENGROK Black Hole Simulation Lab (`blackhole_ds`), performed at the time
> of its relocation into `Z:\[Vibe Coded (AI)]\CLAUDECODE\Black Hole Simulation Lab`
> and its relicense to **Proprietary — All Rights Reserved** (ADR-0009, ADR-0010).
> Reviewer lens: principal-engineer / CTO / scientific-computing due diligence.
> Verdict band: **A− / "publishable research instrument, pre-GPU."**

---

## 0. Executive summary

This is a genuinely strong, scientifically literate C++20 general-relativity
rendering lab. It is **not** a toy. It computes exact analytic black-hole
observables, bends light by integrating real geodesics, and renders the
gravitationally lensed accretion disk and the asymmetric Kerr shadow — each
output tagged with an explicit scientific *truth tier* so beauty never gets
mistaken for a physics claim. The build is green and **17/17 tests pass**
(verified this session, ~30 s wall clock).

What carries it above the median "physics side project":

- A **compile-time dimensional-units type system** (`units.hpp`) that makes
  "add metres to seconds" a *compilation error*, with Kahan summation for
  conserved quantities and analytic validators.
- A **Carter-separated Kerr null-geodesic engine in Mino time** — the
  mathematically correct way to integrate spinning-black-hole photon orbits
  (decouples r and θ, removes turning-point sign flips). This is the part most
  hobby projects get wrong or avoid.
- The **closed-form Bardeen 1973 Kerr shadow** with the exact √27 M
  Schwarzschild reduction as an automated correctness anchor.
- A **governance spine** most research code lacks: ADRs, a Scientific
  Integrity Charter, an ERD/data model, complexity notes, a research
  source-card corpus, CI, clang-format gate, and `.gitattributes` EOL policy.

The headline gaps are honest and known: no GPU renderer yet (the stated goal),
no GRMHD/numerical-relativity, single-threaded CPU renders, and an MSVC-vs-MinGW
toolchain split that CI papers over. None are correctness defects; they are
scope and performance frontiers.

---

## 1. What this is, and what it is good for

**What it is:** a header-only C++20 physics kernel plus a thin CLI. The kernel
(`include/blackhole_ds/`) is organized as `core/` (constants, truth labels),
`metrics/` (analytic Schwarzschild + Kerr observables), `integrators/`
(fixed RK4 and adaptive Dormand–Prince 5(4) with FSAL), `geodesics/`
(Schwarzschild photon orbit, closed-form Kerr shadow, full Kerr null geodesic),
`viz/` (ASCII shadow, PPM shadow/photon-ring, lensed Doppler-beamed disk, Kerr
asymmetric shadow, frame-dragged Kerr disk), and `data/` (CSV export). Around
this sits a research operating system: a brain/soul reasoning-lens corpus (20
scientist XML profiles + XSD), a source-card corpus (20 foundational papers
1828–2025 + JSON Schema + JSONL RAG index), Python validators, and ~52
markdown docs.

**What it is good for, by audience:**

| Audience | Utility |
|---|---|
| Researcher / grad student | A correct, readable reference for Schwarzschild & Kerr observables, light bending, shadows, and lensed-disk imaging, with literature citations inline. |
| Educator | Truth-tier labels make it a teaching instrument about epistemic honesty in simulation, not just the physics. |
| Engineer | A clean example of compile-time units, embedded-RK error control, and ADR-driven governance in modern C++. |
| Future author (you) | A validated CPU seed for the intended GPU ray-marcher; the geometry kernel is the reusable asset. |

**Levels / maturity (CMMI-ish):** code = level 4 (measured, tested, gated);
governance/docs = level 4–5 (defined and continuously improved via ADRs);
product/distribution = level 2 (single CLI, no packaging/releases yet). It is a
**research instrument**, explicitly *not* a validated production solver — the
LICENSE and Integrity Charter both say so, correctly.

---

## 2. Novelty & uniqueness

The individual physics results are textbook (Bardeen 1972/73, Carter 1968,
Cunningham 1975, Luminet 1979, Dormand–Prince 1980) — correctly so; this is a
faithful implementation, not a new theory. **The novelty is in the synthesis
and the discipline**, which is what the proprietary license now protects:

1. **Truth-tier labeling as a first-class, enforced primitive** (`TruthLabel`
   enum threaded through CSV export and every render's stdout banner). Most
   simulators have no machine-readable epistemic status at all.
2. **A reasoning-lens "brain/soul" corpus** (schema-validated scientist
   profiles) coupled to a research source-card RAG index — an unusual
   knowledge-engineering layer wrapped around a numerics kernel.
3. **The combination** of compile-time dimensional safety + Mino-time Kerr
   geodesics + closed-form shadow + governance, in one solo-authored repo, is
   rare. Any one is common; all four together is distinctive.

Bottom line: not novel *physics*, but a novel, defensible *engineered artifact*
— consistent with the All-Rights-Reserved posture.

---

## 3. Is it "too much for humans to grasp"? Team sizing

No — it is comprehensible, by design. The truth-tier discipline, the
literature citations in every header, and the ARCHITECTURE/HIERARCHY docs make
it learnable. The hard parts (Kerr geodesics, the units template
metaprogramming) demand specific background but are well-commented.

**Who can run/extend it:**

- **Maintain & extend on CPU:** 1–2 people with C++17/20 + undergraduate GR
  and numerical-methods background. The current state is a healthy solo-author
  scope.
- **Push to the GPU goal (CUDA ray-marcher):** add 1 GPU/graphics engineer.
- **Aspirational GRMHD / numerical-relativity:** that is a 3–6 person,
  multi-year research effort (a different project class — Einstein Toolkit /
  BHAC territory). The repo is honest that this is out of current scope.

So: **1 today, 2–3 to reach the GPU milestone, a funded team for GRMHD.**

---

## 4. Bugs, problems, issues, errors

No correctness defects were found in the physics or the build; all tests pass.
Findings are graded P1 (should fix) → P3 (polish).

- **[FIXED this session · P1] README status drift.** The "Not implemented yet"
  block claimed the geodesic integrator was "finishing M1" and that there was
  "no disk, no colour yet" — both contradicted by the shipped, tested Kerr
  geodesic engine and the Doppler-beamed disk renderer. This is exactly the
  kind of accidental untruth the Scientific Integrity Charter forbids.
  Corrected to reflect reality (star-field background and GPU port remain the
  real open items).
- **[FIXED this session · P1] License inconsistency surface.** Source SPDX tags
  (`AGPL-3.0-or-later`) now match the new proprietary LICENSE (`LicenseRef-
  Proprietary-AllRightsReserved`) across all 31 C/C++ files; NOTICE, README,
  CONTRIBUTING, HIERARCHY, and integration docs updated. See ADR-0009.
- **[P2] Toolchain split: MSVC (CI) vs MinGW/GCC (local).** `units.hpp` already
  documents that `std::abs/std::sqrt` are rejected in `constexpr` by MSVC but
  accepted by GCC, and works around it. The local build dir is GCC/Makefiles;
  CI is MSVC. Risk: a contributor's local green can be CI-red. *Recommendation:*
  add a local MSVC build profile (or document `-T host=x64` / Ninja+cl) and a
  CI GCC job so both toolchains are exercised on both sides.
- **[P2] `--steps` lower bound only.** Parser enforces `steps >= 1` but no upper
  bound; `--steps 100000000` would print a huge table (DoS-by-foot-gun, not a
  security issue for a local CLI). Cap or document.
- **[P3] Branch hygiene.** A local branch `0thernes/Nautilus` points at a bare
  "Initial commit"; `Nautilus/` is also gitignored. Likely a stale worktree
  artifact — confirm and prune to avoid confusion.
- **[P3] Render guards are honest but lossy.** Both redshift functions clamp
  `denom < 0.05` "for rendering only." Correctly labeled, but a `--physical`
  mode that refuses to clamp (and instead reports the true blueshift) would be
  a nice rigor switch.
- **[P3] PPM-only output.** No PNG without an external `magick` convert step.
  Fine for a kernel; a small stb-style PNG writer would lower friction.

---

## 5. Efficiency, time & complexity

The algorithmic complexity is documented (`docs/engineering/COMPLEXITY.md`) and
the integrators are appropriate (adaptive DP5(4) where stiffness matters, fixed
RK4 where the step is known safe). The real cost center is rendering:

- **Kerr disk render is O(W·H·S²·max_steps)** with a full geodesic integrated
  per sub-sample. Measured this session: disk E2E test ~11.7 s, Kerr shadow
  ~4.2 s, shadow image ~3.3 s — all **single-threaded**.
- **Biggest quick win: parallelize the pixel loop.** The render loops in
  `disk_image.hpp` / `kerr_disk_image.hpp` are embarrassingly parallel and
  side-effect-free per pixel. A single `#pragma omp parallel for` (or
  `std::for_each(std::execution::par, …)`) over rows would give a near-linear
  multicore speedup (≈10–16× on the reference 275HX) for ~3 lines of code, and
  is the natural stepping stone to the CUDA port.
- **Memory:** trivial (per-ray `State<N>` on the stack; one `Image` buffer). No
  leaks possible — no manual `new`/`delete`, all RAII/`std::vector`.
- **Build time:** header-only kernel recompiles per TU; fine at this size, but
  watch it as `viz/` grows — consider a compiled library target later.

No accidental quadratic blowups, no N+1 patterns, no unbounded recursion. The
`max_steps` caps and turning-point reflections prevent runaway integration.

---

## 6. The cross-platform / browser / mobile questions (honest answer)

Several of the brief's questions assume a **web app**. This is not one — it is a
native C++ command-line program that writes PPM image files. So:

- **Web browsers (all types/versions), Flexbox, Bootstrap CSS, "fit any
  screen":** **Not applicable.** There is no HTML, CSS, or JS in the repo, and
  nothing renders in a browser. If a browser-based viewer is desired, that is a
  *new* deliverable (e.g., compile the kernel to **WebAssembly** via Emscripten
  and draw to a `<canvas>`; *then* responsive/Flexbox/mobile questions become
  real). Recommended as a future "M-web" milestone, not a defect today.
- **Desktop OS portability (C++ build):** Windows ✅ (CI on `windows-latest`/
  MSVC; local MinGW). macOS / Linux ✅ *by construction* — the code is standard
  C++20 + `<cmath>` only, the CMake is generator-agnostic, and the README ships
  Linux/macOS build commands. It has not been **CI-verified** on those OSes;
  add Ubuntu + macOS jobs to the GitHub Actions matrix to make the claim
  evidence-backed rather than plausible.
- **Mobile (Android/iOS):** not targeted. The kernel *could* be cross-compiled
  (it's pure C++), but there is no app shell. Out of scope unless/until the
  WASM or native-mobile path is chosen.

So the accurate framing: **portable C++ core, single desktop CLI surface, no
web/mobile surface yet.** That is the right scope for a pre-GPU research kernel;
the brief's UI questions point at a *future* presentation layer.

---

## 7. Scalability & extensibility

Strong foundations for growth:

- **Module boundaries are clean** (`core`/`metrics`/`integrators`/`geodesics`/
  `viz`/`data`), namespaced (`blackhole_ds::…`), header-only with stable
  contracts — easy to add a `gpu/` or `web/` sibling without disturbing the
  kernel.
- **Integrations are ADR-gated and OFF by default** (`BHDS_ENABLE_*` CMake
  options that warn loudly until wired) — a disciplined seam for the tsotchke
  ecosystem.
- **The data model exists** (`data/schema.sql` star schema) ahead of need; the
  roadmap item to add a truth-tier column is the right next data step.
- **To scale further:** (a) split the header-only kernel into a compiled
  `libblackhole_ds` once compile time bites; (b) introduce a `Renderer`
  interface so CPU and future CUDA backends are swappable; (c) add a job/scene
  description (TOML/JSON) so renders are reproducible artifacts, not CLI flags;
  (d) CI matrix across OSes + both toolchains.

Nothing in the architecture blocks scaling; the work is additive.

---

## 8. Documentation, history, licensing, IP

- **Documentation: excellent for the size.** 52 markdown docs — vision,
  mission, integrity charter, ARCHITECTURE/HIERARCHY, ERD/ERM, COMPLEXITY,
  ROADMAP/KANBAN, ADRs, daily log, research program + source cards, two prior
  audits, a 500-point inspection. README is thorough with rendered example
  images. This is **top-decile** for a solo research repo.
- **History/logs:** ADRs + DAILY_LOG + DECISIONS + CHANGELOG give a real
  paper trail. The ADR discipline (never renumber, record process failures —
  see ADR-0006's MIT-vs-AGPL correction) is exemplary governance.
- **Licensing & IP — now correct and consistent.** Per your directive: **NOT
  MIT.** The repo is now **Proprietary — All Rights Reserved** (ADR-0009): root
  LICENSE rewritten (reserves all rights, limited source-viewing grant only, no
  patent grant, no-ML-training clause, contributions by assignment, warranty &
  liability disclaimers, governing-law + severability, §9 relicensing path);
  NOTICE rewritten; all 31 source SPDX tags switched to
  `LicenseRef-Proprietary-AllRightsReserved`; downstream docs aligned.
  Historical audit docs intentionally left stating "AGPL" as point-in-time
  record.
- **Remaining doc nits:** (a) `CITATION.cff` would help if you ever cite this
  in academic work; (b) a top-level `CHANGELOG` entry for this relicense/move
  (added); (c) Next-Milestones item 1 in README is now historical (geodesic
  integrator done) — minor.

---

## 9. Prioritized recommendations

| # | Priority | Action | Effort |
|---|---|---|---|
| 1 | P1 ✅ done | Relicense to All Rights Reserved; align all SPDX tags + docs | done this session |
| 2 | P1 ✅ done | Fix README status drift (integrity) | done this session |
| 3 | P2 | Parallelize render pixel loop (OpenMP / `std::execution::par`) | hours; ~10–16× speedup |
| 4 | P2 | CI matrix: add Ubuntu + macOS, and a local MSVC profile | hours |
| 5 | P2 | Cap `--steps`; add `--physical` no-clamp render mode | small |
| 6 | P3 | Prune stale `0thernes/Nautilus` branch / worktree | minutes |
| 7 | P3 | Add PNG writer + scene-file (TOML/JSON) for reproducible renders | small–medium |
| 8 | Roadmap | CUDA ray-marcher backend behind a `Renderer` interface | the headline goal |
| 9 | Roadmap | Optional WebAssembly/`<canvas>` viewer → unlocks the browser/mobile/responsive questions | medium |

---

## 10. Sign-off

The engine is correct, tested, well-governed, and now correctly licensed as
proprietary novel IP. It is ready to be the consolidated CLAUDECODE-portfolio
home for the black-hole work and a solid base to build the GPU renderer on. The
only "stale" items were a drifted README block (fixed) and the license surface
(fixed). No blocking bugs. Recommended next concrete step: the multicore render
parallelization (item 3) — highest value for least effort, and the on-ramp to
the GPU goal.

*— Tier-1 review, 2026-06-13. Empirical basis: full source read of the kernel,
CLI, and tests; clean `cmake --build`; `ctest` 17/17 pass in 30.4 s.*
