# Contributing to BlackHoleDS / OPPENGROK

This is a research-first, scientifically-honest, type-safe C++ black hole
simulation lab. Contributions are welcome but the bar is real. Please read
this document and the architecture decision records in
`docs/log/DECISIONS.md` before your first PR.

## Project Rules

1. **Scientific honesty.** Every value, plot, table, exported row, and
   visualization must declare a `model_status` label from the list in
   `docs/architecture/ARCHITECTURE.md`. Frontier physics ideas are welcome
   but must be labeled `speculative_extension` and live in clearly separated
   modules. Never present approximations as exact results.
2. **Local Git is the source of truth.** No more ZIP-over-the-wall. Submit
   work as commits and pull requests. ZIPs and patches must be archived
   under `_incoming/` (ignored) and applied with a documented script.
3. **Units safety.** Physical quantities use the strong types in
   `include/blackhole_ds/units.hpp`. Raw `double` in physics code is a
   review-blocker unless explicitly justified.
4. **No secrets, ever.** Never commit credentials, API keys, tokens, or
   anything that resembles one. The `.gitignore` rules are the floor, not
   the ceiling.
5. **Validation gates.** `python scripts/Validate-ResearchOS.py` must pass
   locally before every commit. CI runs the same check on every push.

## Daily Workflow

```powershell
cd "Z:\Orca\Workspaces\Stress Test Agents Maxxxing"
git pull --rebase origin main

# Make your change. Always start a feature branch for non-trivial work.
git switch -c feature/short-name

# Validate
python scripts/Validate-ResearchOS.py

# Build (when CMake is installed)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure

# Commit
git add -A
git commit -m "Short imperative subject, then body if needed."

# Push and open a PR
git push -u origin HEAD
gh pr create --fill
```

For tiny, low-risk changes (docs, single-line fixes), committing directly to
`main` is acceptable, but only after validation + tests pass locally.

## Commit Message Style

- First line: imperative, under 70 characters. ("Add Schwarzschild CSV
  exporter" not "Added an exporter".)
- Then a blank line.
- Then a wrapped paragraph or short bullets describing the why, not the what.
- Mention the iteration number if it is part of the rolling plan.

## Pull Request Style

- Link the relevant ADR (or open one) for architecture-level changes.
- Include a short test plan, even for documentation changes.
- Keep PRs focused. One logical change per PR beats a megabranch.

## What Belongs Where

- `src/`: C++ implementation.
- `include/blackhole_ds/`: public headers and stable contracts.
- `tests/`: CTest targets, smoke + integration + property tests.
- `scripts/`: build/dev/research helpers (Python, PowerShell).
- `tools/`: standalone data-science helpers (Python harness, etc.).
- `data/`: schemas and curated reference data.
- `docs/architecture/`: how the system is built.
- `docs/research/`: source cards and research notes.
- `docs/log/`: daily log, decision records.
- `knowledge/brains/`: scientist reasoning-lens XML profiles.
- `knowledge/papers/`: paper/source index for the RAG layer.
- `external/`: third-party integrations (submodules or `FetchContent`).
- `assets/diagrams/`: source Mermaid/SVG/PlantUML files.
- `.oppengrok/`: agent coordination files (claim files, swarm board).

## What Does Not Belong in Git

- Build outputs (`build/`, `*.o`, `*.exe`, `*.dll`).
- Generated data (`exports/`, `*.sqlite`, generated `*.csv`).
- Local agent state (`.orca/`, `.codex/`, Orca worktrees, daemon PIDs).
- Secrets of any kind. If you accidentally commit one, stop, rotate the
  secret upstream first, then we will rewrite history.

## Questions

If something is ambiguous, open a small issue and propose an ADR. Doing the
right thing slowly beats doing the wrong thing fast.
