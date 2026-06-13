# BlackHoleDS — Inspiration & Operating Philosophy Briefing
## For All 32 Parallel Agents, Ralph Wiggum Daemons, and Human Contributors

**Version**: v0.1 (2026-05-25) — Living document. Updated by daemons on major philosophy or audit changes.
**Sources (read these first)**:
- Maximum Seeking v2.5 (6 Pillars, 20 Sections, ~150 Principles, 11 Frameworks, Via Negativa, Phase-Separated Cognition, Calibrated Optimism, Conflict Resolution Protocol, Failure Modes Catalog)
- Gold Standard Code Review & Audit SOP v6.0 (35 Dimensions D01-D35, Hard Floors on D01 Security & D20 Compliance, 5 Maturity Tiers, DORA integration, AI disclosure rules)
- "GROK is TERRY DAVIS" framework (TempleOS-level obsessive correctness, from-scratch rigor, no shortcuts, intellectual honesty as moral duty)
- The 104 Greatest Developers, Programmers, Engineers & Architects across 7 decades (Turing, von Neumann, Knuth, Stroustrup, Torvalds, Carmack, Karpathy, Bellard, etc. — their actual methods, not myths)

This briefing translates those sources into **non-negotiable operating rules** for this specific project: a C++-primary visual black hole simulation platform that is simultaneously a first-class data science instrument (SQL, DAX, Power BI, Excel, Graph Theory, Type Theory, Chaos, Temporal Quantum, Cosmology).

---

## 1. Truth & Reality (Pillar I) — Non-Negotiable

- Every conserved quantity (E, Lz, Carter Q) **must** be cross-checked analytically at runtime (Kahan + validators in units.hpp). If the numbers drift, the simulation is lying. We admit it immediately.
- Analytic limits (photon sphere exactly 3 rg for Schwarzschild, ISCO 6 rg, shadow ~5.2 rg) are **deductive truth anchors**, not suggestions. The C++ engine and the Python harness must agree to machine epsilon or the audit fails (points 101-125).
- "It looks right on the plot" is never evidence. Reproducibility + analytic match + ensemble statistics + cross-validation against EHT numbers for Sgr A* / M87* are the only currency.

## 2. First Principles & Type Theory (Pillar VI + 104 Greats)

- **No raw doubles in physics code.** The units.hpp strong typedef system (Quantity<LengthTag>, etc.) + user-defined literals (`10.0_rg`, `1.0_M`) is the law. Adding meters to seconds must be a compile error. This is Stroustrup + Knuth + modern C++ concepts done correctly.
- Every module must be decomposable to irreducible axioms (the metric, the geodesic equation, the Lyapunov definition, the causal set construction). No "magic numbers" or "it worked in my test."
- Via Negativa: when we revise theory/ or src/physics/, we must remove at least as many lines of approximation or special cases as we add. Complexity is a liability near horizons.

## 3. Phase-Separated Cognition (Maximum Seeking Tension Resolution)

- **Generation phase** (new integrator, new chaos diagnostic, new visualization encoding, new DAX measure): emotional, metaphorical, oblique, Synectics, dreams, "what if we treated the event horizon as a persistent homology problem?" Judgment OFF. Wild ideas welcome.
- **Selection / Validation phase**: System 2 only. Falsification, analytic cross-check, 250-point audit, best-of-n tournaments (see best-of-n skill), statistical power analysis on ensembles. Judgment ON. No emotional attachment to a clever trick that fails the validators.

## 4. Gold Standard SOP v6.0 — Every Dimension Applies

- D01 (Security) & D20 (Compliance) have **hard floors of 8.0**. No exceptions. The .gitignore + MCP secret scanning gate + Ralph daemon pre-commit hook are not bureaucracy — they are the price of admission for a research instrument that will be used by real scientists.
- AI-generated code (including anything this subagent or main agent produces) must be tagged, verified, and held to the identical standard as human code. No free pass.
- The 250-point audit is not a one-time document. It is a living ledger. Every commit that touches code must reference the specific points it advances or closes.

## 5. The 104 Greatest — Concrete Methods We Emulate Here

- **Knuth**: Literate programming in comments, exhaustive test oracles against analytic solutions, TAOCP-level attention to numerical stability (Kahan, condition numbers, compensated summation).
- **Stroustrup**: Zero-overhead abstractions, RAII, value semantics, strong types that make invalid states unrepresentable.
- **Torvalds / Bellard**: Small, reviewable, obviously correct kernels. The geodesic integrator should be readable in one sitting by a competent physicist who knows C++.
- **Carmack / Karpathy**: Data-driven validation. Every visual must be traceable to a row in the SQLite fact table. "Trust the pixels only because you can query the photons."
- **von Neumann / Chandrasekhar**: The mathematics is the source of truth. Code is a faithful executor of the mathematics, never the other way around.

## 6. Ralph Wiggum Daemons & 32-Agent Coordination — How We Actually Achieve "No Mistakes"

- The daemons (Orca scheduler + GitHub Actions + in-process supervisors) exist because human + single-agent attention is insufficient for a project of this scope and required precision.
- When a daemon or parallel subagent finds a violation (secret, physics drift, spelling, audit point regression), it does not "suggest." It opens the issue, references the exact 250-point number, and (where safe) proposes the minimal diff that restores compliance.
- Best-of-n tournaments are mandatory for any numerical kernel that affects conserved quantities or published observables (shadow diameter, Lyapunov, jet power proxies).
- 32 agents do not mean 32 independent copies of the same work. Work is partitioned by the architecture (one agent owns the units system + validators, another owns the data schema + DAX, another owns the lensing ray-marcher, another owns the Ralph daemon loop itself, etc.). Results are compared; winners are merged; losers become regression tests.

## 7. Visuals, Data Science, and the Inductive/Deductive Duality

- The pretty pictures (colorful, high-contrast, accessible, scientific palettes) are **diagnostic views** into the data, never the primary artifact.
- Every exported chart, every Power BI measure, every Excel pivot must be reproducible from the canonical SQLite star schema + the exact git commit + seed.
- Inductive mode (ensemble statistics, causal discovery on 10,000 runs, symbolic regression hints on chaos vs jet power) and deductive mode (pure GR forward integration from the metric) must be forced to confront each other on every major release. When they disagree, the deductive side wins until the induction reveals a real missing term in the theory.

## 8. Practical Operating Rules for This Repo

1. Never commit without running the local secret scanner + spell-check + at least the unit validators from units.hpp.
2. Every new source file or major refactor gets at least 3 dedicated 250-point audit items written at the time of creation.
3. Spelling, grammar, or naming inconsistency is an automatic audit point (category 10). Zero tolerance.
4. The proprietary, All-Rights-Reserved license is intentional (see ADR-0009). This research instrument is novel work; all rights are reserved to the Author. Any open release is a deliberate, version-scoped decision, never a silent default.
5. When in doubt, return to the analytic formula. Then write the test that would have caught the doubt.

---

**"The hallmark of intellectual honesty is recognizing how easy it is to be ignorant of your own ignorance, and committing to seek and refine without end."**

— Maximum Seeking v2.5

**"You can do it. All night long."**

— The user who created this stress-test workspace, and every one of the 104 Greatest who ever stayed up until 4 a.m. because the photons were not yet right.

This briefing is now part of the 250-point audit (points 26-50 and the philosophy alignment category). It will be re-read by every new agent and every Ralph daemon cycle.

*End of Inspiration Briefing v0.1*
