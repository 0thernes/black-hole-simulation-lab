# Daily Log

A short, dated, append-only record of what changed in the project each day.

Use this for the operational story: what you worked on, what surprised you,
what you learned, what is blocked. Detailed engineering notes belong in
`docs/reports/PROJECT_LOG.md`. Architecture choices belong in `DECISIONS.md`.

Entry format:

```text
## YYYY-MM-DD

- Context: one short line about the day's focus.
- Done: bulleted list of concrete changes (link commits if useful).
- Learned: anything physics, math, or engineering you noticed.
- Next: the next concrete step.
- Open questions: anything you want to come back to.
```

---

## 2026-05-26

- Context: Reset the project to a single source of truth on disk after the
  ChatGPT ZIP-over-the-wall era. Establish daily commit and push cadence.
- Done:
  - Iteration 1: Foundation sanity. Fixed `BlackHoleDS.cpp` to use the
    canonical units header, sober banner, cleaner `.gitignore`. Build and
    smoke tests pass on `g++ 14.2 (MinGW UCRT)`.
  - Iteration 2: Repo skeleton. Added module directories for
    `src/{core,metrics,integrators,data,cli}` and matching headers under
    `include/blackhole_ds/`. Added `knowledge/brains/<category>/`,
    `knowledge/papers/`, `external/`, `assets/diagrams/`, and the VS Code
    workspace. Added governance files: CODEOWNERS, CONTRIBUTING, SECURITY.
- Learned: GPT's ZIPs never actually applied. The local repo is small but
  clean, and `tsotchke/eshkol` plus the rest of the tsotchke ecosystem
  (`quantum_geometric_tensor`, `spin_based_neural_network`, `libirrep`,
  `PINN`, `quantum_rng`, `moonlab`) are highly relevant downstream
  integrations.
- Next: Iteration 3 (vision, mission, scientific integrity charter, ERD).
- Open questions:
  - Which Eshkol or tsotchke components do we wire first?
  - Do we keep MIT or switch to AGPLv3 for the project license?
