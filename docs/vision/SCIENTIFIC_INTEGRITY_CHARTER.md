# Scientific Integrity Charter

This document is the project's scientific constitution. Every contributor
and every AI agent operating on this codebase is bound by it. Violating
this charter is a review-blocker.

## I. The Six Truth Tiers

Every value, plot, table, exported row, validated test, and visualization
in this project must carry one of these `model_status` labels:

| Tier | Label | Meaning |
|---|---|---|
| T1 | `analytic_classical` | Exact closed-form result derived from classical general relativity (Schwarzschild, Kerr, BPT formulas, etc.). Must match the analytic formula to machine precision (typically < 1e-12 relative error). |
| T2 | `numerical_approximation` | Result from a numerical method (geodesic integrator, GRMHD solver, etc.) with documented error bounds. |
| T3 | `observational_constraint` | Value anchored in published observational data with citation, uncertainty, and date (EHT M87* ring, LIGO GW150914, etc.). |
| T4 | `visualization_metaphor` | Visual produced for explanation, not measurement. Not a physics claim. |
| T5 | `pedagogical_simplification` | Simplified or approximate derivation chosen for clarity. Limitations stated up front. |
| T6 | `speculative_extension` | Frontier idea (Hawking radiation interior, Page curve, fuzzballs, islands, primordial BH constraints, holographic complexity, etc.). May be wrong. Must not contaminate T1-T3 modules. |

**The cardinal rule:** never let a T6 value sit in the same table as a T1
value without an unmistakable label.

## II. Source Discipline

- Every implemented formula must be traceable to a citation in
  `docs/research/source_cards/`.
- Every observational constraint must include the publication, date,
  reported value, and reported uncertainty.
- Every frontier extension must declare what would falsify it (or label
  itself as not currently falsifiable, which is fine but must be explicit).

## III. Units and Numerics

- All physical quantities use the strong types from
  `include/blackhole_ds/units.hpp`. Raw `double` in physics code is a
  review-blocker unless it is dimensionless and explicitly justified.
- Conserved quantities (energy, angular momentum, Carter constant) are
  accumulated with `KahanAccumulator` and cross-checked against analytic
  invariants at every meaningful integration milestone.
- Every numerical method declares its order, its stability domain, and its
  expected error scaling with step size.

## IV. Reproducibility

- Every reported result must be reproducible from:
  - the source commit hash,
  - the exact CLI invocation,
  - the random seed (if any),
  - and the recorded model parameters.
- Two identical invocations must produce identical outputs, modulo
  documented numerical tolerances.

## V. Honest Failure Modes

- "I do not know" is a respectable answer.
- "We cannot model this yet" is a respectable answer.
- "This is wrong, here is what I missed" is a respectable answer.
- Hand-waving over a gap, or labeling a speculation as a result, is not.

## VI. AI-Assisted Code and Documents

- AI-generated code is welcome, but the human committing it owns it. If
  you cannot explain a line, do not commit it.
- AI-generated documents are welcome, but every citation must be verified
  against the cited source. AI-fabricated references are a review-blocker.
- The reasoning-lens XML profiles under `knowledge/brains/` are not claims
  that this system contains a person's mind. They are structured prompts
  that help us ask better questions in the style of a known thinker. We
  do not put words in real people's mouths.

## VII. Speculation, Frontier, and Future Physics

This project does not promise discoveries. If the lab is ever used to
publish or discuss new physics:

- The claim must survive a documented, reproducible derivation.
- The claim must be cross-validated against at least one independent
  numerical or analytic method.
- The claim must be reviewed by at least one human with relevant domain
  expertise before it leaves the repository.

## VIII. Charter Updates

This charter changes via an Architecture Decision Record (ADR) in
`docs/log/DECISIONS.md`. Edits without an accepted ADR are reverted.
