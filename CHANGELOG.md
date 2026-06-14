# Changelog

All notable project changes should be recorded here. Keep this human-readable;
use `docs/reports/PROJECT_LOG.md` for detailed operational notes.

## 2026-06-14 (Adversarial audit remediation + Eshkol/Tsotchke capability study)

A multi-agent, adversarially-verified audit (34 raw findings → 8 confirmed real)
plus a source-grounded study of the Eshkol/Tsotchke ecosystem. Fixes:

- **`integrators/rk45.hpp` — false-failure bug fixed.** The `min_step` abort
  could fire on the step proposed *after* the integration already reached `t1`
  (a deliberately tiny final clamped step), returning `success = false` with the
  correct state in `y`. Added an endpoint-reached check immediately after step
  acceptance, before the `min_step` floor. Verified by a new regression in
  `tests/integrator_tests.cpp` (lands on `t1` via a 1e-14 final step → success).
- **`integrators/ode_state.hpp` — divide-by-zero floor.** `weighted_rms_norm`
  divided by `scale_i = atol + rtol*|y|`, which is `0` when `abs_tol == 0` and a
  component is identically zero (→ NaN poisons the controller). Floored the
  weight; new regression asserts a finite norm in that configuration.
- **`metrics/kerr.hpp` — spin clamp widened to the physical `[-1, 1]`.** ISCO
  and photon-sphere clamped spin to `0.999`, silently mis-reporting valid
  near-extremal spins (the schema admits up to `0.9999`; ~9% ISCO error). The
  BPT closed form is finite and exact at extremal (`isco(1)=1`, `isco(-1)=9`,
  `photon(1)=1`); new `smoke_tests.cpp` checks cover this.
- **Truth-tier / doc-accuracy corrections** (Scientific Integrity Charter):
  the Kerr-disk header and `docs/images/README.md` no longer claim the disk
  tracer's conserved-quantity drift "is bounded and tested" (those tests cover
  the *second-order* integrator; the disk uses a first-order on-shell form);
  the disk "shadow boundary … is exact" line now correctly reads
  numerical horizon-capture (distinct from the closed-form Bardeen boundary in
  `kerr_shadow.hpp`); `kerr_geodesic.hpp` now cross-references that the disk
  ray tracer does **not** ride on its second-order integrator; ROADMAP M5's
  stale "Kerr disk ray trace remain[s]" parenthetical corrected.
- **Integration plans corrected to match upstream source** (`ESHKOL_INTEGRATION.md`,
  `TSOTCHKE_ECOSYSTEM.md`): Eshkol has no `.es → .hpp` transpiler (it compiles
  to native via LLVM) and is MIT, not GPL/AGPL; `quantum_geometric_tensor` is a
  *quantum*-state (Fubini-Study) library, not a GR Christoffel/Riemann backend
  (category error removed); `libirrep` (MIT) remains the correct first adapter.
  Net: the classical-GR kernel needs none of these on its critical path; the
  one genuine reuse is `differential_geometry.c` as a verification *oracle*.

Build clean (0 warnings), **17 CTest suites green**, format + validation gates pass.

## 2026-06-13 (Relicense to proprietary + workspace relocation + Tier-1 audit)

Three coupled changes, all recorded in ADRs.

- **Relicense → Proprietary, All Rights Reserved** (ADR-0009, supersedes the
  AGPL-3.0 decision in ADR-0006). Root `LICENSE` rewritten as a proprietary
  All-Rights-Reserved grant (limited source-viewing only; no use/copy/modify/
  redistribute; no patent grant; no-ML-training clause; contributions by
  assignment; warranty + liability disclaimers; governing-law/severability;
  §9 future-relicensing path). `NOTICE` rewritten. **All 31 C/C++ source files**
  re-tagged `SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved`
  with an "All Rights Reserved" copyright line. README, CONTRIBUTING,
  HIERARCHY, TSOTCHKE_ECOSYSTEM, ESHKOL_INTEGRATION, and INSPIRATION_BRIEFING
  updated; dependency-compatibility calculus inverted (permissive deps OK,
  copyleft deps now incompatible without a commercial license). Historical
  audit docs left stating "AGPL" as point-in-time record.
- **Relocation** (ADR-0010): canonical working tree moved from
  `Z:\Orca\Workspaces\Stress Test Agents Maxxxing` to
  `Z:\[Vibe Coded (AI)]\CLAUDECODE\Black Hole Simulation Lab`. Git history and
  the `origin` remote are unchanged — local path only.
- **Tier-1 full audit** added at
  `docs/audits/AUDIT-2026-06-13-RELOCATION-AND-TIER1-REVIEW.md` (verdict A−:
  publishable research instrument, pre-GPU). Fixed a README status-drift
  integrity bug (the "Not implemented yet" block contradicted the shipped Kerr
  geodesic engine and Doppler-beamed disk). Top recommendation: parallelize the
  render pixel loop (≈10–16× quick win, on-ramp to the GPU port).

Verified: build clean, 17/17 CTest pass (30.4 s).

## 2026-06-12 (Remediation batch 3: accuracy + referential integrity)

More inspection FAILs closed, weighted toward "gates that gate".

- S14.13: corrected source-card `where_used` paths — three pointed at the
  empty `src/BlackHoleDS.cpp` shim; now point at the real headers
  (`metrics/schwarzschild.hpp`, `metrics/kerr.hpp`).
- S14.12 + S14.14 (new gates): `validate_source_cards.py` now enforces
  cross-corpus referential integrity — every `related_brains` slug must
  resolve to a real brain profile, and every claim `where_used` path must
  exist on disk. These prevent the S14.13 class of drift from recurring.
- S06.11: `constants.hpp` now *derives* `geometric_meters_per_solar_mass`
  from the IAU 2015 nominal `GM_sun = 1.3271244e20 m^3/s^2` and `c^2`
  (anchoring on the well-measured GM_sun rather than the poorly-known
  product G*M_sun), with a `static_assert` pinning the result to ~1476.6 m.
- S11.03: HIERARCHY.md no longer claims the `.mmd` diagrams live under
  `assets/diagrams/` — they are under `docs/architecture/diagrams/`.
- S11.05: SYSTEM_DIAGRAM.md's layer responsibilities now say the physics
  kernel is header-only under `include/` today (not `src/`).
- S02.11: enabling a `BHDS_ENABLE_*` integration flag now emits a CMake
  `message(WARNING)` that the adapter is not wired yet, so the option is
  honest instead of a silent no-op.

Verified: build clean, 7/7 CTest pass, source cards regenerate and pass the
new referential-integrity gates, full validation green.

## 2026-06-12 (Remediation batch 2: tests + licensing)

Burning down more inspection FAILs, focused on test coverage and AGPL
enforceability.

Testing (S07.05, S07.06, S07.08, S07.18):
- Extracted the CLI parser into `include/blackhole_ds/cli/options.hpp` so
  it is unit-testable in isolation (pure: argv in, Options + diagnostics
  out, no globals).
- `tests/cli_tests.cpp`: 14 grouped checks covering defaults, valid flags,
  --help, unparseable values, missing values, unknown format, --steps < 1,
  unknown args, strict trailing-garbage rejection, plus CsvWriter header
  exactness, truth-label prefix, 17-digit precision, and stream-flag
  restoration.
- CTest grew from 3 to 7 cases: added the CLI unit suite and output-
  asserting runs (PASS_REGULAR_EXPRESSION on the text banner and the CSV
  header; a WILL_FAIL case for a bad flag). The bare run no longer passes
  on exit code alone.
- S03.18: corrected the `--steps` help text — it is the number of spin
  intervals, so the table has steps+1 rows (default 9 -> 10 rows).

Licensing (S19.09, S19.10, S19.11):
- Added `SPDX-License-Identifier: AGPL-3.0-or-later` + copyright headers to
  all 15 C++ source/header files.
- Added root `NOTICE` (AGPL notice + pointers) and `AUTHORS` (copyright
  holders).
- CONTRIBUTING.md gained a "Licensing of Contributions" section stating the
  inbound=outbound AGPL model and the DCO sign-off encouragement.

Verified: clang-format idempotent on all 15 files, build clean, 7/7 CTest
suites pass, validation green.

## 2026-06-13 (M5: the frame-dragged Kerr lensed accretion-disk image)

The payoff — the gravitationally lensed accretion disk around a **spinning**
black hole, ray-traced through real Kerr null geodesics. The far side of the
disk is bent up and over the shadow (the Interstellar/EHT warp), one side is
Doppler-beamed bright, and frame dragging twists the geometry and shifts the
inner shadow — all from integrating the actual geodesic, not a planar
approximation.

- `viz/kerr_disk_image.hpp`: per-pixel backward ray tracing. Each image
  coordinate (alpha, beta) → constants (E, L_z, Q) → an ingoing photon
  integrated in Mino time, watching for the equatorial plane (disk crossing,
  with higher crossings = lensed images), horizon capture (shadow), or escape
  (background). The integrator uses the **first-order on-shell form**
  (dr/dλ = s_r √R, dθ/dλ = s_θ √Θ recomputed each step, sign flips at genuine
  turning points) — robust against the off-shell drift that the second-order
  form suffers over the huge observer-to-hole radius range. Disk hits are
  coloured by the **Kerr circular-orbit redshift factor**
  `g = 1/[u^t(1 - Ω b)]` (g⁴ beaming + temperature shift).
- CLI `--kerr-disk <file.ppm>` (uses `--spin`, `--inclination`).
- `tests/kerr_disk_tests.cpp`: the central anchor is an **a → 0 regression** —
  with zero spin the Kerr tracer reproduces the validated Schwarzschild disk
  tracer (`viz/disk_image.hpp`) across a grid (95.5% hit-kind agreement, 90%+
  disk-radius agreement; remaining differences are sub-pixel disk-edge and
  lensed-ring boundaries), plus the Kerr redshift → Schwarzschild redshift
  reduction, shadow/disk presence at high spin, and frame-dragging shadow
  displacement. New `blackhole_ds_kerr_disk_tests` target; registered in the
  validation gate. **17 CTest suites, all green.**
- `docs/images/kerr_disk_a09_i78.png`: reference render (a/M = 0.9, i = 78°).
- Truth tiers: lensing geometry/trajectory `numerical_approximation` (geodesic
  integration, conserved-quantity-checked); redshift factor `analytic_classical`;
  emissivity/colour a `visualization_metaphor`.

## 2026-06-13 (M5: Kerr null-geodesic integrator — the spinning-disk engine)

The engine the spinning-disk ray trace will ride on. A Schwarzschild photon
stays in a plane (`d2u/dphi2 + u = 3u^2`); a Kerr photon does not — frame
dragging twists it out of plane — so this integrates the full geodesic in
Boyer-Lindquist coordinates using the separated constants of motion.

- `geodesics/kerr_geodesic.hpp`: the Carter-separated equations of motion in
  **Mino time**, in second-order form `d²r/dλ² = ½R'(r)`,
  `d²θ/dλ² = ½Θ'(θ)`. Mino time decouples r and θ into 1-D "particle in a
  potential" problems, and the second-order form removes the square root —
  so there is **no sign flip to track at the radial/polar turning points**,
  which is exactly where naive first-order √R integrators break. Cyclic
  `dφ/dλ`, `dt/dλ` from the first integrals. Constants: E, L_z, Carter Q
  (Q = 0 ⇔ equatorial). Composes with the existing `rk4_step`. Plus
  `radial_residual`, `polar_residual`, `recover_Q` for verification.
- `tests/kerr_geodesic_tests.cpp`: verified the way GR integrators must be —
  by conserved quantities. (1) Q and the on-shell residuals do not drift
  along a generic 3-D orbit (a=0.5, E=1, L_z=2, Q=3). (2) a → 0 reproduces
  the Schwarzschild photon circle r=3 at b=√27 (R=R'=0, stays circular).
  (3) the equatorial plane is geodesic (θ=π/2 stays put). (4) frame dragging:
  a zero-angular-momentum photon advances in φ for a>0 but not a=0. (5) an
  **independent cross-check** — the prograde equatorial photon radius from the
  closed-form shadow module (`kerr_shadow.hpp`) is exactly where this
  integrator finds R=R'=0, and a photon launched there stays on the circle.
- New `blackhole_ds_kerr_geodesic_tests` target; registered in the validation
  gate. **16 CTest suites, all green.**
- Observer image-plane → constants-of-motion map (Cunningham-Bardeen 1973):
  `constants_from_image(alpha, beta, i, a)` gives `L_z = -alpha sin i`,
  `Q = beta^2 + (alpha^2 - a^2) cos^2 i`, and `initial_state_from_image`
  builds the on-shell ingoing photon at a far observer radius — the bridge
  that turns each pixel into a geodesic to trace. Tested by the
  self-consistency identity `Theta(i) == beta^2`, the a → 0 form, on-shell
  residuals, and an **exact round-trip** against the closed-form shadow
  boundary (a boundary point maps back to its orbit's L_z = xi, Q = eta).
- Truth tier: the equations of motion are `analytic_classical` (Carter 1968);
  the trajectories are `numerical_approximation` (RK4) with bounded,
  test-checked conserved-quantity drift. No CLI/image surface yet — this is
  the foundation for folding frame dragging into the lensed-disk ray trace.

## 2026-06-13 (M5 proper: the Kerr (spinning) black-hole shadow)

The first true *spin* physics: the asymmetric, D-shaped shadow of a rotating
black hole — the defining visual signature of angular momentum and the
quantity the Event Horizon Telescope uses to constrain spin.

- `geodesics/kerr_shadow.hpp`: the shadow boundary as the closed-form
  projection of the unstable spherical photon orbits (Bardeen 1973). Conserved
  quantities `xi(r) = -(r^3 - 3r^2 + a^2(r+1))/(a(r-1))` and
  `eta(r) = r^3(4a^2 - r(r-3)^2)/(a^2(r-1)^2)`; celestial coordinates
  `alpha = -xi/sin i`, `beta = +/- sqrt(eta + a^2 cos^2 i - xi^2 cot^2 i)`
  swept over the equatorial photon-orbit range `[r_prograde, r_retrograde]`.
  No numerical geodesic integration — the boundary is exact (`analytic_classical`).
- `viz/kerr_shadow_image.hpp`: rasterises the D-shaped silhouette (black) with
  a thin photon-ring rim (a `visualization_metaphor`); `shadow_alpha_extent`
  quantifies the spin displacement.
- CLI: `--kerr-shadow <file.ppm>` (reuses `--spin` and `--inclination`),
  reporting the horizontal alpha-extent and its asymmetry.
- `tests/kerr_shadow_tests.cpp`: validates the equatorial photon-orbit radii
  (3M at a=0; 1M / 4M at a=1), the **a -> 0 reduction to the exact sqrt(27) M
  circle**, the alpha-asymmetry + beta-symmetry of the D-shape at high spin,
  and point-in-shadow behaviour. New `blackhole_ds_kerr_shadow_tests` target +
  a `--kerr-shadow` CLI smoke test: **15 CTest suites, all green.**
- `docs/images/kerr_shadow_a099_i80.png`: reference render at a/M = 0.99,
  i = 80 deg (horizontal alpha-extent [-2.28, 6.95] M, asymmetry 4.67 M vs 0
  for Schwarzschild). Registered in the validation gate.
- Honesty: the shadow boundary is exact GR (Bardeen 1973), anchored by the
  tested Schwarzschild limit. The photon-ring rim is decorative. The lensed
  *disk* image remains Schwarzschild — frame dragging is not yet in the disk
  ray trace (next increment).

## 2026-06-13 (Render polish: supersampling anti-aliasing)

- `viz/disk_image.hpp`: `render_disk_image` now averages `samples x samples`
  sub-pixel rays (default 2x2) via a new `sample_colour` helper, removing the
  jagged shadow/disk edges. `DiskView.samples` controls the rate; the
  geometry and physics are unchanged, so all symmetry/redshift tests still
  hold (they exercise `trace_disk_pixel` directly). 13 suites green.
- Regenerated the `docs/images` reference renders with anti-aliasing.

## 2026-06-13 (M5 start: Doppler beaming + gravitational redshift)

The disk image now carries the famous *brightness* asymmetry — one side
bright, the other dim — from real relativistic physics, not a painted-on
gradient.

- `viz/disk_image.hpp`: added `redshift_factor(r, X, sin_i)`, the exact
  Schwarzschild thin-disk frequency ratio
  `g = sqrt(1 - 3M/r) / (1 + r^{-3/2} X sin i)` for material on circular
  geodesic orbits — numerator is the gravitational + orbital time dilation
  (1/u^t), denominator is the longitudinal Doppler with `X sin i = -L_z/E`
  (Cunningham-Bardeen). Observed flux scales as g^4 (Liouville), so the
  approaching side is beamed bright and the receding side dimmed; the
  colour temperature is shifted by g. Reduces to no-Doppler at face-on.
- `tests/disk_tests.cpp`: added redshift-factor checks — face-on g is
  X-independent and equals sqrt(1-3/r); inclined is asymmetric (one side
  > grav, the other <); no emission inside 3M; and the rendered inclined
  image is left-right brightness-asymmetric while the geometry stays
  left-right symmetric. 13 suites, all green.
- Regenerated `docs/images/lensed_disk_i80.png` / `i20.png`: they now show
  the EHT M87*-style bright/dim crescent in addition to the lensing warp.
- Honesty upgrade: the redshift factor g is now GR-exact
  (`analytic_classical`); only the emissivity profile and colour ramp
  remain `visualization_metaphor`. Kerr spin / frame dragging is still M5
  proper. README, CLI message, and image docs updated accordingly.

## 2026-06-13 (M3: the iconic lensed accretion-disk image)

The first gravitationally lensed black-hole image — the canonical
"Interstellar/EHT" look — by backward ray tracing on the M1 geodesics.

- `include/blackhole_ds/viz/disk_image.hpp`: renders a thin equatorial
  accretion disk (r_in..r_out) seen at observer inclination i. For each
  pixel it integrates the photon orbit u(phi) and finds where the orbit
  plane crosses the equatorial disk plane (g(phi) = cos(phi)cos i -
  sin(phi)(Y/b)sin i = 0); the radius at each crossing decides disk hit
  (front + higher-order lensed images), shadow (captured), or background.
  The far side of the disk lenses up and over the shadow — the iconic warp.
- `tests/disk_tests.cpp`: validates the exact geometric symmetries that
  must hold — face-on (i=0) is rotationally symmetric (depends only on b);
  inclined is mirror-symmetric left<->right and asymmetric top<->bottom;
  the centre is the shadow; every disk hit lands in [r_in,r_out]; a far
  corner is background.
- `src/cli/main.cpp`: `--disk <file.ppm>` + `--inclination <deg>`.
- CMake: `blackhole_ds_disk_tests` + a `--disk` CTest. 13 suites, all green.
- `docs/images/`: committed PNG reference renders (lensed disk at 80 deg
  and 20 deg, plus the bare shadow + photon ring), generated by the CLI
  and shown at the top of the README. Satisfies the M3 reference-image
  exit criterion.
- Honesty: the lensing geometry is numerical_approximation on the
  validated geodesic; colour is a visualization_metaphor. No Doppler or
  gravitational redshift yet (that is M5), so brightness is left-right
  symmetric — only the *shape* warp is real lensing.

Capability: solidly on rung 3 of the visual ladder — a recognizable,
physically-grounded lensed black-hole image. Next: GPU port (M4), then
Doppler/redshift + Kerr spin (M5).

## 2026-06-13 (First raster image: shadow + photon ring to PPM)

The first real raster (pixel) image, building on the M1 geodesics.

- `include/blackhole_ds/viz/ppm_writer.hpp`: a dependency-free RGB `Image`
  with binary (P6) and ASCII (P3) PPM writers. Pure infrastructure; every
  future raster output (CPU lensing, GPU render) uses it.
- `include/blackhole_ds/viz/shadow_image.hpp`: renders the Schwarzschild
  shadow encircled by its photon ring. The shadow (b < sqrt(27) M) is
  black; the ring brightness is driven by the *actual integrated
  deflection angle* delta(b) from the validated geodesic module — bright
  where light winds near the photon sphere, fading to a dim sky floor far
  out. Honest tiering: shadow radius is analytic_classical; the brightness
  ordering follows the numerical deflection; the colour map itself is a
  visualization_metaphor (not a radiometric flux calculation).
- `tests/render_tests.cpp`: PPM P6/P3 byte-level checks, out-of-bounds
  safety, ring-brightness ordering (shadow dark < far field < ring), and
  the headline validation — the shadow radius measured FROM the rendered
  401px image matches sqrt(27) M to within one pixel.
- `src/cli/main.cpp`: `--image <file.ppm>` writes the render and prints the
  measured vs analytic shadow radius. View the PPM in any image viewer
  (GIMP, IrfanView, ImageMagick `magick shadow.ppm shadow.png`).
- CMake: `blackhole_ds_render_tests` plus an `--image` CTest (11 suites
  total, all green). New files registered in the validation gate.

Capability: a stronger rung-3 step than the ASCII shadow — a true pixel
image whose ring structure comes from real geodesic bending. Next (M3):
lens an accretion disk / background so the iconic asymmetric image appears.

## 2026-06-13 (First image: the black-hole shadow)

The project draws its first picture of a black hole.

- `include/blackhole_ds/viz/ascii_shadow.hpp`: renders the Schwarzschild
  shadow as ASCII. Each image-plane cell (x, y in units of M) maps to a ray
  of impact parameter b = sqrt(x^2 + y^2); the validated geodesic
  classifier decides captured (shadow) vs escaping (sky). The shadow is the
  exact photon-capture disk of radius sqrt(27) M ~ 5.196 M, with the rim
  drawn as a photon-ring band.
- `tests/shadow_tests.cpp`: verifies the shadow radius equals b_crit,
  membership at the centre/rim/exterior, isotropy, and that the rendered
  captured-pixel area matches the analytic disk area pi*b_crit^2 within 3%.
  CTest is now 9 suites.
- `src/cli/main.cpp`: `--shadow` flag prints the ASCII shadow with truth
  tiers (the silhouette is visualization_metaphor; the radius is
  analytic_classical). `blackhole_ds --shadow` shows a round shadow ringed
  by the photon band.

Capability ladder: this is the simplest form of rung 3 (a shadow image),
built entirely from the validated geodesic physics - no fudged graphics.
Next toward the full visual: a lensed background and an accretion disk,
then the GPU port.

## 2026-06-13 (Milestone M1: first geodesics - the engine bends light)

The jump from "calculator" to "can trace a ray" - the gateway to every
visual capability.

- `include/blackhole_ds/geodesics/schwarzschild_photon.hpp`: equatorial
  null geodesics in Schwarzschild, via the photon orbit equation
  d^2u/dphi^2 + u = 3u^2 (u = M/r) integrated by the existing RK4 stepper.
  Provides: the critical impact parameter b_crit = sqrt(27) M (the shadow
  boundary), ray classification (escapes / captured / critical), the photon
  turning point via a robust bisection, and the total light-deflection
  angle.
- `tests/geodesic_tests.cpp`: validates against closed-form results - the
  weak-field deflection converges to the Eddington 4M/b (0.3% at b=1000,
  3% at b=100, with the correct positive GR correction), b_crit = 3*sqrt(3),
  capture below critical, NaN handling, and the turning point solving its
  defining equation to 1e-12. CTest is now 8 suites.
- `src/cli/main.cpp`: new `--deflection <b/M>` flag. `blackhole_ds
  --deflection 5.5` reports a 146-degree bend near the photon sphere;
  `--deflection 3` reports CAPTURED. Real, runnable, truth-tier-labeled
  output that demonstrates light bending around a black hole.

This places the project at rung 2 of the capability ladder (single
geodesic integration). Next: assemble many rays into a CPU shadow/lensing
image (M3).

## 2026-06-12 (Phase C: 500-point inspection + remediation batch 1)

- `docs/audits/INSPECTION-500-POINT.md`: a 25-section, 500-point grounded
  inspection. Each section was inspected by an independent agent that read
  the actual files (run in waves to respect rate limits). Honest result
  distribution: 333 PASS (67%), 110 PARTIAL, 41 FAIL, 14 TODO, 2 N/A — not
  a wall of PASS, which is the point. Resolves the previously dangling
  links in README and docs/INDEX.md.
- Began burning down the 41 FAIL findings (remediation batch 1):
  - S01.03: removed the phantom `.oppengrok/` line from HIERARCHY.md.
  - S01.08/09: removed 14 redundant `.gitkeep` files from populated dirs;
    kept only the 11 in genuinely empty directories.
  - S12.05: added `CHECK` constraints to `dim_observer` (inclination
    0..90, distance_rg > 0) enforcing the documented domains.
  - S12.16: rewrote `data/schema.sql` header/footer — removed the "Ralph
    Wiggum daemons" meme cruft and fabricated "250-point audit" process
    claims; documented the real ADR-gated change process and the
    truth-tier-column gap honestly.
  - S20.12 (Charter violation): `tools/blackhole_ds_harness.py`
    `lyapunov_estimate` no longer injects Gaussian noise "for realism" —
    it returns None (NULL) until a real estimator exists. Fabricated data
    dressed as measurement is exactly what the Integrity Charter forbids.
  - S16.01/16.18: removed unused imports (typing + openpyxl) and a no-op
    14-iteration column-width loop from the harness.
  - S15.09: broadened the secret-name pattern to catch `app.env`,
    `prod.env`, and `*.env` suffix forms, not just `.env`/`.env.prod`.
  - S08.10: added `.github/dependabot.yml` (weekly github-actions + pip).
- Validation green; harness runs with NULL Lyapunov (no fabricated data).

## 2026-06-12 (Phase D: CI/CD and formatting upgrades)

- `.clang-format` (LLVM base, 4-space, 80-col, no string-literal splitting)
  adopted as canonical; the whole C++ tree reformatted to conform
  (idempotent). ADR-0007.
- `.github/workflows/ci.yml` rebuilt into three jobs:
  - `format`: clang-format `--dry-run --Werror`, version pinned to
    19.1.7 via pip so CI matches the formatting version exactly.
  - `validate`: corpus rebuild + drift gate + all validators, with pip
    caching keyed to requirements-dev.txt.
  - `build`: a Debug + Release matrix (was Release only), needs validate.
  Plus least-privilege `permissions: contents: read`, `concurrency`
  cancellation of superseded runs, and `FORCE_JAVASCRIPT_ACTIONS_TO_NODE24`
  to clear the Node 20 deprecation without guessing action versions.
- `.gitattributes` rewritten with explicit eol policy (LF for source/docs/
  scripts/data, CRLF for `.ps1`, binary for assets). ADR-0008. Files
  renormalized.
- `scripts/dev/Install-PreCommitHook.ps1` now writes the generated sh hook
  with forced LF, so a CRLF checkout can no longer corrupt the shebang
  (audit F-022).
- `scripts/dev/format.ps1`: apply or `-Check` formatting locally with the
  same style CI enforces.
- `requirements-dev.txt`: lxml + jsonschema (turns the optional validation
  paths into hard gates in CI).
- Verified: clang-format idempotent, both Debug and Release build and pass
  3/3 CTest suites, full validation green.

## 2026-06-12 (Phase B: comprehensive documentation suite)

- `docs/INDEX.md`: the documentation map / table of contents for the whole
  docs tree, with role-based reading paths (new contributor, academic, VC,
  engineer).
- `docs/GLOSSARY.md`: physics, numerics, and project-convention terms,
  pitched for a software-strong reader learning GR.
- `docs/architecture/ERM.md`: the conceptual entity-relationship model and
  a full data dictionary (every column: type, unit, constraint, meaning),
  with cardinalities, business rules, the conceptual→logical→physical
  layering, and the open data-model gaps.
- `docs/planning/ROADMAP.md`: milestones M0-M6 with explicit exit criteria,
  a Mermaid flow, and the sequencing rule (physics never outruns evidence).
- `docs/process/KANBAN.md`: a file-based Kanban with column policies, a
  WIP limit, and cards seeded from the roadmap and open audit findings.
- `docs/operations/ENGINEERING_RESOURCE_PLAN.md`: the "ERP" reinterpreted
  for a GPU-driven project - compute/toolchain needs per milestone, who
  can run what, and the VRAM/core capacity reasoning behind the
  laptop-GPU render goal.
- README: added a table of contents, a copy-paste quickstart, a license/CI
  status line, and a documentation map section.
- ARCHITECTURE deepened with the integrators section (in Phase A).
- Validate-ResearchOS.py REQUIRED_FILES extended for the new docs.

## 2026-06-12 (Phase A: numerical integrators + complexity reference)

First real DSA: the ODE machinery the geodesic solver (milestone 1) needs.

- `include/blackhole_ds/integrators/ode_state.hpp`: stack-allocated
  `State<N>` (`std::array`, no heap) with O(N) `add`/`axpy`/`scale` and a
  tolerance-weighted RMS error norm (Hairer-Norsett-Wanner).
- `include/blackhole_ds/integrators/rk4.hpp`: classic fixed-step RK4,
  global error O(h^4), 4 derivative evals/step.
- `include/blackhole_ds/integrators/rk45.hpp`: adaptive Dormand-Prince
  5(4) with the full standard Butcher tableau, embedded 4th-order error
  estimate, elementary step-size controller (h * safety * e^(-1/5)
  clamped), FSAL reuse (6 evals/accepted step), forward and backward
  integration, and a runaway-step guard.
- `tests/integrator_tests.cpp`: verifies RK4 hits e to 1e-10, RK4
  4th-order convergence (empirical error ratio in [12,20] for 2x steps),
  SHO energy conservation over a full period, DP45 accuracy + adaptive
  step counts, that tighter tolerance buys accuracy at the cost of steps,
  and backward integration. Wired into CTest (now 3 suites).
- `docs/engineering/COMPLEXITY.md`: time/space complexity of every
  algorithm in the repo (current and planned), including the
  embarrassingly-parallel-over-pixels scaling argument for the GPU
  render goal and the error-vs-step-count distinction for numerical
  methods.
- ARCHITECTURE.md updated: integrators section, build contract (3 test
  targets), realized module-shape tree.
- Validate-ResearchOS.py REQUIRED_FILES extended for the new artifacts.

Verified: clean MinGW build, all 3 CTest suites pass.

## 2026-06-12 (Remediation: fixes 1-5 from the full audit)

20 audit findings closed, including all 3 criticals. North star reaffirmed:
a visual, GPU-accelerated black hole simulation runnable on RTX-class
consumer hardware.

- **Fix 1 - CI green:** `units.hpp` validators no longer call std-math in
  constant expressions (hand-rolled abs, precomputed sqrt(27) literal) -
  MSVC-clean per the C++20 standard. Added the missing binary
  `operator+`/`-` and scalar `*`/`/` for Quantity (same-tag only), making
  the dimensional-safety claim real; removed the dimensionally broken
  `square()`/`sqrt(Length)` helpers. Compile-time `Addable` concept tests
  prove Length+Length compiles and Length+Time does not.
- **Fix 2 - shadow physics:** corrected the radius/diameter factor-of-2:
  the validator now checks the true Schwarzschild shadow DIAMETER
  2*sqrt(27) M ~ 10.392 M and explicitly rejects the radius (5.196 M)
  passed off as a diameter. Harness `photon_sphere_rg`/`isco_rg` replaced
  with the exact Bardeen-Press-Teukolsky forms matching `metrics/kerr.hpp`
  (verified value-for-value); the invented shadow spin polynomial (wrong
  sign) is gone. CSV export now writes max_digits10 (17 significant
  digits) so exports round-trip at the project's 1e-12 validation level.
- **Fix 3 - license:** ADR-0006 records the truth: the project license is
  and has been AGPL-3.0. ADR-0004 marked superseded with a process-failure
  note (it was written from a chat description instead of the actual
  file). HIERARCHY, TSOTCHKE_ECOSYSTEM, and ESHKOL_INTEGRATION corrected.
- **Fix 4 - determinism:** corpus generators read created/updated dates
  from the seed JSON instead of the wall clock; consecutive regenerations
  are byte-identical (verified). All 20 sources gained explicit metadata
  blocks. `audit.ps1` now regenerates BEFORE validating. CI gained a
  corpus drift gate (`git diff --exit-code` after regeneration).
- **Fix 5 - gates that gate:** `Validate-ResearchOS.py` REQUIRED_FILES
  now covers all iteration 2-8 artifacts (57 files). `brain_soul.xsd`
  year type changed from `xs:gYear` to a YearOrPeriod pattern that
  legitimately admits "1960s"/"1900-1930s"-style scholarly periods;
  `validate_brains.py` mirrors the pattern, scans for orphan XMLs, and
  runs full lxml XSD validation when available (CI installs lxml).
  `validate_source_cards.py` enforces `schemas/source_card.json` via
  jsonschema (CI installs it) - and immediately caught a real bug: the
  schema's year minimum (1900) rejected Gauss 1828; widened to 1600.
  README rewritten: GPU-visual north star, current layout tree, honest
  implemented/not-implemented lists, milestone ladder ending at the CUDA
  ray-marched visual prototype. INITIAL_250 audit doc marked historical.
- Audit document updated: 20 findings flipped to FIXED, 47 remain OPEN.

## 2026-06-12 (Full repository audit)

- `docs/audits/AUDIT-2026-06-12-FULL-REPO-REVIEW.md`: multi-agent audit of
  the entire repo at commit `6e7cff1`. Seven dimension reviewers (C++,
  physics/math, Python, PowerShell, docs, schemas/data, git/CI); every
  candidate finding independently re-verified by an adversarial agent that
  read the cited files. 69 findings raised, 67 confirmed, 2 refuted.
- Confirmed: 3 critical, 28 major, 30 minor, 6 info.
- The three criticals: (1) CI has never been green - all recorded runs
  failed; current cause is MSVC rejecting the constexpr validators in
  `units.hpp` (std::sqrt/std::abs are not constexpr-valid under MSVC
  C++20, while MinGW GCC accepts them as builtins, so local builds pass
  and CI fails). (2) The "shadow diameter ~5.2 rg" quantity in
  `units.hpp` and the Python harness is actually the shadow *radius*
  (b_crit = sqrt(27) M); the diameter is ~10.39 M - a factor-of-2 error
  in a headline observable. (3) The harness `photon_sphere_rg()`
  approximation deviates up to ~97% from the exact Kerr formula now in
  `metrics/kerr.hpp` while being labeled "high-fidelity".
- Also confirmed: the root LICENSE file is AGPL-3.0, contradicting
  ADR-0004 and three docs that claim MIT. The license question needs an
  explicit decision and a corrected ADR.
- Remediation is tracked finding-by-finding in the audit document
  (Status: OPEN per finding).

## 2026-05-26 (Iteration 8: Daily workflow automation)

Bootstrap-plan completion. Daily forward motion now has a single command.

- `scripts/dev/build.ps1`: CMake configure + build with the MinGW
  toolchain auto-detection. `-Configuration Debug|Release`, `-Clean`.
- `scripts/dev/test.ps1`: CTest with `--output-on-failure`.
- `scripts/dev/audit.ps1`: runs the Research OS validator and regenerates
  the brain and source-card corpora when their seeds exist.
- `scripts/dev/clean.ps1`: wipes `build/`, `exports/`, and
  `_incoming/extracted/`.
- `scripts/dev/Daily-Commit.ps1`: validate -> build -> test -> commit ->
  push. Refuses to commit without a message or push without `-Push`.
- `scripts/dev/Install-PreCommitHook.ps1`: installs a local pre-commit
  hook that runs the Research OS validator before every commit.
- `docs/process/DAILY_WORKFLOW.md`: full reference for the loop.
- `README.md`: replaced the manual workflow section with the new wrapper
  scripts plus a manual fallback.
- `.github/workflows/ci.yml`: now mirrors local validation by running the
  brain and source-card builders/validators alongside the existing
  CMake/CTest steps. CI and the daily workflow gate on the same checks.
- Removed obsolete `scripts/dev/.gitkeep` since the directory now has
  content.

Verified: `audit.ps1` regenerates both corpora, `test.ps1` runs CTest 2/2
green, and the pre-commit hook installer writes a working `.git/hooks/pre-commit`.

This commit closes the eight-iteration bootstrap plan. The repo now has:
foundation sanity, repo skeleton, vision/mission/integrity charter, ERM
diagrams, 20-profile brain corpus, 20-source research corpus, tsotchke
integration plan with ADR-0005, a modular C++ kernel with CLI and CSV
output, and an automated daily workflow.

## 2026-05-26 (Iteration 7: C++ modularization)

- Split the monolithic `src/BlackHoleDS.cpp` into focused modules:
  - `include/blackhole_ds/core/constants.hpp`: SI constants, solar mass,
    geometric-units conversion factor.
  - `include/blackhole_ds/core/truth_label.hpp`: compile-time enum mirroring
    the six Scientific Integrity Charter tiers, with `to_string` view.
  - `include/blackhole_ds/metrics/schwarzschild.hpp`: analytic Schwarzschild
    radius, photon sphere, and ISCO in SI meters.
  - `include/blackhole_ds/metrics/kerr.hpp`: dimensionless Kerr ISCO and
    photon-sphere closed forms.
  - `include/blackhole_ds/data/csv_writer.hpp`: truth-tier-aware CSV emitter.
  - `src/cli/main.cpp`: real CLI entry point with `--mass`, `--spin`,
    `--format text|csv`, `--steps`, and `--help`.
- `src/BlackHoleDS.cpp` is now an empty translation unit that preserves the
  existing CMake target name while the real entry point lives under `src/cli/`.
- `tests/smoke_tests.cpp` rewritten with a CHECK macro so tests work
  identically in Debug and Release (no more silent `assert()` strip).
  Tests now cover the new module headers, truth-label stringification,
  and Schwarzschild/Kerr canonical values, including monotonicity checks
  for spin direction.
- `CMakeLists.txt`: links `src/cli/main.cpp` into the `blackhole_ds` target.
- Verified: clean build, no warnings, `ctest` 2/2 passed, CLI flags and
  CSV output work as documented.

## 2026-05-26 (Iteration 6: Tsotchke ecosystem integration plan)

- `docs/integrations/TSOTCHKE_ECOSYSTEM.md`: full map of the tsotchke
  repository roster with role, language, stars, and integration tier.
  Tier A (this iteration: documentation only), Tier B (target Q2:
  `libirrep`), Tier C (target Q3: `eshkol` and
  `quantum_geometric_tensor`), Tier D (target Year 2+: `moonlab`, `PINN`,
  `quantum_rng`).
- `docs/integrations/ESHKOL_INTEGRATION.md`: five-stage rollout plan for
  using Eshkol as the DSL layer for metric definitions, with build
  pipeline, risks, and parity-test gates.
- `external/README.md`: adapter pattern rules. Adapter-only wrappers,
  commit-SHA pins, opt-in CMake options, ADR-gated adoption.
- `external/tsotchke/` directory placeholder for future adapters.
- `CMakeLists.txt`: opt-in options
  `BHDS_ENABLE_LIBIRREP / ESHKOL / QGT / MOONLAB` added with commented
  `FetchContent_Declare` example. All default OFF; default build path is
  unchanged.
- `docs/log/DECISIONS.md`: ADR-0005 codifies the external-integration
  policy (opt-in, SHA-pinned, adapter-only, ADR-gated).

Verified: build, tests, and Research OS validation all still pass with
the new options.

## 2026-05-26 (Iteration 5: Research foundation - source cards)

- `schemas/source_card.json`: JSON Schema for the source-card system.
  Required fields: slug, kind, title, authors, year, model_status, claims,
  metadata. Truth-tier enumeration matches the Scientific Integrity Charter
  and the brain XML schema. Claims include statement, tier, optional
  equation, value, uncertainty, and where_used routing.
- `knowledge/papers/seed_sources.json`: 20 foundational sources spanning
  1828 to 2025:
    analytic_classical (12): Gauss 1828 (Theorema Egregium), Schwarzschild
      1916, Newman-Penrose 1962, Kerr 1963, Penrose 1965, Penrose 1969,
      Bardeen-Press-Teukolsky 1972, MTW 1973, Bardeen-Carter-Hawking 1973,
      Bekenstein 1973, Schoen-Yau 1979, Chandrasekhar 1983, Choquet-Bruhat
      1952.
    observational_constraint (5): Ghez 2008, LIGO GW150914 2016, EHT M87*
      2019, EHT Sgr A* 2022, GWTC-4 2025.
    speculative_extension (2): Hawking 1974, Strominger-Vafa 1996.
- `scripts/research/build_source_cards.py`: deterministic generator that
  emits one Markdown card per source, plus `knowledge/papers/INDEX.jsonl`
  for cheap RAG lookup and `docs/research/source_cards/INDEX.md`.
- `scripts/research/validate_source_cards.py`: structural validator that
  checks slugs, tiers, required sections, and INDEX consistency.
- `scripts/Validate-ResearchOS.py`: refactored to share a sub-validator
  helper and now also runs the source-card validator when the corpus is
  built. Brain and source-card corpora gate together.

## 2026-05-26 (Iteration 4: Brain/Soul XML system)

- `schemas/brain_soul.xsd`: XML Schema for the reasoning-lens profiles.
  Enforces required sections (identity, contributions, reasoning_lens,
  how_to_apply, metadata), the category enumeration, and the truth-tier
  enumeration for bibliography model statuses.
- `knowledge/brains/seed_profiles.json`: source of truth for the seed
  roster. 20 deep profiles total: 9 physicists (Schwarzschild, Kerr,
  Einstein, Penrose, Hawking, Thorne, Newman, Bekenstein, Strominger),
  6 mathematicians (Riemann, Poincare, Gauss, Cartan, Choquet-Bruhat, Yau),
  5 astronomers (Chandrasekhar, Eddington, Ghez, Genzel, Doeleman). Each
  profile carries real contributions, year, relevance to BlackHoleDS,
  proof standard, favorite questions, failure modes, bibliography with
  truth-tier labels, and module-relevance routing.
- `scripts/brains/build_brains.py`: deterministic generator that emits one
  XML file per profile under `knowledge/brains/<category>s/<slug>.xml`,
  plus `knowledge/brains/MANIFEST.json` and `knowledge/brains/INDEX.md`.
- `scripts/brains/validate_brains.py`: structural validator. Checks
  parseability, schema location, slug-filename match, category-directory
  match, required sections, and INDEX.md count consistency.
- `scripts/Validate-ResearchOS.py`: now runs the brain validator
  automatically if the manifest exists. Allows staged adoption.
- Generated 20 XML profiles, MANIFEST.json, and INDEX.md. Validation passes.

## 2026-05-26 (Iteration 3: Vision, mission, and ERM documents)

- `docs/vision/VISION.md`: long-term aim, what the project is, what it is
  not, and a three-year horizon.
- `docs/vision/MISSION.md`: 12-month mission with six pillars and quarterly
  objectives for Q1 through Q4.
- `docs/vision/SCIENTIFIC_INTEGRITY_CHARTER.md`: project constitution with
  the six truth tiers (analytic_classical, numerical_approximation,
  observational_constraint, visualization_metaphor, pedagogical_simplification,
  speculative_extension), source discipline rules, units and numerics rules,
  reproducibility rules, and AI-assisted code policy.
- `docs/architecture/ERD.md` + `docs/architecture/diagrams/erd.mmd`: human
  and Mermaid views of the canonical SQLite star schema.
- `docs/architecture/SYSTEM_DIAGRAM.md` +
  `docs/architecture/diagrams/system_pipeline.mmd`: end-to-end pipeline
  from research layer through contracts, kernel, data, visualization, and
  audit.
- `docs/architecture/diagrams/module_dependency.mmd`: target C++ module
  dependency graph for after Iteration 7.

## 2026-05-26 (Iteration 2: Repo skeleton for daily work)

- Added the module directory skeleton matching the architecture doc:
  `src/{core,metrics,integrators,data,cli}/` plus matching headers under
  `include/blackhole_ds/`.
- Added `knowledge/brains/{mathematicians,physicists,astronomers,coders,
  developers,engineers,architects,scientists}/` to hold the seven 100-strong
  reasoning-lens XML rosters that arrive in Iteration 4.
- Added `knowledge/papers/`, `schemas/`, `external/`, `assets/diagrams/`,
  `docs/{integrations,log,vision,research/source_cards}/`,
  `scripts/{dev,brains,research}/`, and `docs/architecture/diagrams/`.
- Added governance files: `CODEOWNERS`, `CONTRIBUTING.md`, `SECURITY.md`.
- Added `docs/log/DAILY_LOG.md` (operational story) and
  `docs/log/DECISIONS.md` (architecture decision records with ADRs 0001-0004).
- Added `docs/architecture/HIERARCHY.md` describing the live directory map
  and ownership semantics.
- Added VS Code workspace under `.vscode/`: settings, recommended extensions,
  build/test tasks for both g++ (immediate) and CMake (after install), and
  debugging configs.

## 2026-05-26 (Iteration 1: Foundation sanity)

- Rewrote `src/BlackHoleDS.cpp` to use the canonical `blackhole_ds::units`
  types from `include/blackhole_ds/units.hpp` instead of redefining its own
  `Length`, `Time`, and `Dimensionless` structs. Eliminates a quiet type-system
  divergence between the executable and the smoke tests.
- Replaced the chatty seed banner with a professional, sober output that still
  emits scientifically-honest `model_status` labels for every printed table.
- Rewrote `.gitignore` to use whole-name patterns for secret detection instead
  of substring matches like `*token*` and `*auth*` that would block legitimate
  filenames (`token_parser.hpp`, `authentication.cpp`). Allow-listed curated
  research data and diagrams that must stay tracked.
- Verified `g++ 14.2` (MinGW UCRT, via CodeBlocks) builds both targets and that
  smoke tests pass. CMake install is queued.

## 2026-05-26

- Established the local Git repository as the source of truth.
- Added direct local package intake workflow and hardened package selection.
- Added CMake, CTest, CI, and Research OS validation scaffolding.
- Added architecture, research, testing, workflow, gameplan, and project-log docs.
- Organized source philosophy/audit inputs under `docs/source/`.
- Added the existing C++ seed, SQLite schema, Python harness, and research docs to
  the committed project baseline.
