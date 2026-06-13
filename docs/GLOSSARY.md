# Glossary

Physics, mathematics, and project-specific terms used throughout the repo.
Aimed at a reader who is strong in software but learning general relativity
(the author's stated position), and at a GR-fluent reader new to the
project's conventions.

## Physics and general relativity

- **Geometric units.** A unit system with G = c = 1, so masses, lengths,
  and times share dimensions. A length in these units is measured in
  `rg = GM/c^2`. For one solar mass, `rg ≈ 1476.625 m`. The repo uses
  geometric units internally and converts to SI only at the I/O boundary.
- **Schwarzschild radius (`r_s`).** `2GM/c^2 = 2 rg`. The event horizon of
  a non-rotating black hole.
- **Photon sphere.** The radius of unstable circular photon orbits. For
  Schwarzschild, `3 rg = 1.5 r_s`. Light here can orbit the black hole.
- **ISCO (innermost stable circular orbit).** The smallest radius at which
  a massive particle can orbit stably. Schwarzschild: `6 rg = 3 r_s`.
  Marks the inner edge of a thin accretion disk.
- **Shadow.** The dark region an observer sees, bounded by photons that
  asymptote to the photon sphere. The Schwarzschild shadow **radius** is
  the critical impact parameter `b_crit = √27 rg ≈ 5.196 rg`; the
  **diameter** is `2√27 rg ≈ 10.392 rg`. (Confusing these is the
  factor-of-2 error the audit caught; the codebase now guards it.)
- **Impact parameter (`b`).** The perpendicular distance from the black
  hole to the asymptotic line of an incoming light ray. Rays with
  `b > b_crit` escape; `b < b_crit` are captured; `b = b_crit` asymptote
  to the photon sphere.
- **Kerr metric.** The exact solution for a rotating, uncharged black hole.
  Parameterized by mass `M` and spin `a = J/(Mc)` (dimensionless `a/M` in
  `[-1, 1]`; negative = retrograde).
- **Ergosphere.** The region outside a Kerr horizon where no observer can
  remain stationary (frame dragging forces co-rotation). Site of the
  Penrose energy-extraction process.
- **Carter constant.** A fourth conserved quantity (beyond energy, axial
  angular momentum, and rest mass) that makes Kerr geodesics integrable.
- **Gravitational lensing.** The bending of light by spacetime curvature.
  Weak-field deflection by a mass is `4GM/(c^2 b)`.
- **Doppler beaming.** Apparent brightening of disk material moving toward
  the observer (and dimming of receding material); produces the
  characteristic bright/dark asymmetry in black-hole images.
- **Gravitational redshift.** Light climbing out of a gravitational well
  loses energy, shifting toward the red.
- **Hawking radiation.** Predicted thermal emission from black holes due to
  quantum effects near the horizon. Temperature `T = ℏc^3/(8πGMk_B)`.
  Theoretically established but **unobserved**; tagged
  `speculative_extension` in this repo.
- **Bekenstein-Hawking entropy.** Black-hole entropy proportional to
  horizon area: `S = k_B c^3 A / (4 ℏ G)`.
- **GRMHD.** General-Relativistic MagnetoHydroDynamics — the simulation of
  magnetized plasma in curved spacetime; the regime of production
  accretion-disk codes (BHAC, iharm3D). A long-term aspiration, not a
  current capability.

## Numerical methods

- **ODE state.** The vector of quantities integrated forward; for a 4D
  geodesic it is 8 components (4 position + 4 momentum).
- **Runge-Kutta (RK).** A family of one-step ODE integrators. RK4 is the
  classic fixed-step 4th-order member.
- **Dormand-Prince 5(4).** An embedded RK pair (the "ode45" method): a
  5th-order solution plus a 4th-order estimate, whose difference estimates
  the local error and drives adaptive step sizing.
- **FSAL (First Same As Last).** A property where the last stage of one
  step equals the first stage of the next, saving one derivative
  evaluation per accepted step.
- **Adaptive step size.** Adjusting the integration step to keep the
  estimated error near a target, spending compute where the solution is
  hard (e.g. near the photon sphere) and saving it where smooth.
- **Kahan summation.** Compensated summation that bounds floating-point
  round-off to `O(ε)` instead of `O(Mε)` over M additions. Used for
  conserved quantities over long integrations.

## Project conventions

- **Truth tier / model_status.** One of six labels every published value
  must carry: `analytic_classical`, `numerical_approximation`,
  `observational_constraint`, `visualization_metaphor`,
  `pedagogical_simplification`, `speculative_extension`. Defined in
  `docs/vision/SCIENTIFIC_INTEGRITY_CHARTER.md`.
- **Brain / Soul profile.** A structured XML "reasoning lens" in the style
  of a known thinker (e.g. Schwarzschild, Kerr). A prompt-engineering aid,
  **not** a claim to contain a person's mind.
- **Source card.** A structured record of one research source, with
  truth-tier-labeled claims, used to anchor every implemented formula.
- **ADR (Architecture Decision Record).** A short, numbered, append-only
  record of a hard-to-reverse decision, in `docs/log/DECISIONS.md`.
- **Drift gate.** A CI check (`git diff --exit-code` after regenerating
  the corpora) that fails if committed generated files diverge from what
  the deterministic generators produce.
- **`rg`.** Shorthand for the gravitational radius `GM/c^2`, the natural
  length unit. Many columns are suffixed `_rg`.
- **OPPENGROK.** The project's working codename (the repository is
  `Stress-Test-Agents-Maxxxing` on GitHub for historical reasons).
