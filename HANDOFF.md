# Handoff: Gemini Carbon, GPT Astra, Claude Fable

Owner: Alex Donahue (0thernes). Branch `cursor/research-archive-and-starfield-b952`, PR #9.

## Rules (non-negotiable)
- Real physics and math only. No cosmetic features. Visuals are a readout of physics.
- Languages: Eshkol, C, C++ (CUDA C++ for GPU), Python for harness/analysis only. No JS/TS/HTML/WebGL.
- Every feature ships with tests against closed-form or published values; cite equations/arXiv ids in code and docs/PHYSICS.md.
- Label: established GR vs model assumption vs speculative.
- Commit locally and push to GitHub on every change. Do not push to or merge into main without Alex.

## State (2026-11-10)
- Release build 23/23 tests pass (g++ 13). Windows MSVC CI not yet run on this branch.
- Implemented: exact Schwarzschild + Kerr geodesics (RK4), Kerr shadow, lensed disk + Doppler, star field via escape azimuth (Schwarzschild only), Page-Thorne flux (closed form a=0, numeric Kerr), I_obs = g^4 F/pi, Schwarzschild photon-orbit Lyapunov exponent and e^-pi subring demagnification, integer certificates (photon sphere, b^2=27, ISCO E/L, BPT at a=0) in C++ static_assert and proofs/ClassicalGR.lean (Lean not yet run in CI).
- Research archive: knowledge/papers/ARCHIVE-AUDIT.md, reports/INDEX.md, monthly reports 2026-06..10, reports/WHAT-TO-BUILD.md. Feed spans 2026-06-14..2026-10-24 (34 drops); nothing earlier exists.
- Toolchain: quantum_rng linked (POSIX); Eshkol, Moonlab, QGTL, Rosette Wire evaluated and not linked (reasons in docs/PHYSICS.md).

## Open work (physics first)
1. Kerr null geodesics with Carter constant via Mino-time elliptic-integral solutions; cross-check RK4.
2. Kerr escape direction (removes Kerr disk dark floor; enables Kerr star field).
3. Kerr photon-orbit radii, critical curve, Kerr Lyapunov exponents.
4. Wire Page-Thorne flux into the rendered disk (replace the screen ramp).
5. Covariant radiative transfer along rays.
6. CUDA port per ADR-0013 (stage 0 done: device probe + CPU fallback).
7. Run Lean proofs in CI; MSVC CI on this branch.
8. Dependabot: take #8 (lxml security) after the XSD gate; #6 CI-only.
