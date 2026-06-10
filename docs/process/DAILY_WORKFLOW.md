# Daily Workflow

The repo is structured so that the everyday loop is small and the safety
gates are automatic. Mission pillar 4 (daily forward motion) lives or dies
on this loop being frictionless.

## One-shot script

```powershell
.\scripts\dev\Daily-Commit.ps1 -Message "Short imperative subject" -Push
```

That command:

1. Runs `scripts\dev\audit.ps1` — Research OS validation, plus regeneration
   of the brain and source-card corpora if their seeds exist.
2. Runs `scripts\dev\build.ps1` — `cmake -B build` + `cmake --build build`.
3. Runs `scripts\dev\test.ps1` — CTest with `--output-on-failure`.
4. If anything failed, refuses to commit. If clean, stages everything,
   commits with the supplied message, and pushes to `origin/main` only
   when `-Push` is passed.

Skip steps you do not need with `-SkipBuild` or `-SkipTests`. Doc-only
changes skip both safely; the audit step still runs.

## Step-by-step (manual loop)

```powershell
# 1. Refresh from origin.
git pull --rebase origin main

# 2. Edit / add files.

# 3. Validate.
.\scripts\dev\audit.ps1

# 4. Build and test (skip if pure-doc change).
.\scripts\dev\build.ps1
.\scripts\dev\test.ps1

# 5. Inspect.
git status
git diff

# 6. Commit and push.
git add -A
git commit -m "Short imperative subject"
git push origin main
```

## Individual scripts

| Script | Purpose |
|---|---|
| `scripts\dev\build.ps1` | CMake configure + build. `-Configuration Debug`, `-Clean`. |
| `scripts\dev\test.ps1` | Run CTest against the existing build tree. |
| `scripts\dev\audit.ps1` | Research OS validation + corpus regeneration. |
| `scripts\dev\clean.ps1` | Remove `build/`, `exports/`, `_incoming/extracted/`. |
| `scripts\dev\Daily-Commit.ps1` | Validate -> build -> test -> commit -> push. |
| `scripts\dev\Install-PreCommitHook.ps1` | Install a `pre-commit` hook that runs the validation gate. |

## Pre-commit hook

Install once with:

```powershell
.\scripts\dev\Install-PreCommitHook.ps1
```

After installation, every `git commit` runs the Research OS validation
gate. A failure aborts the commit. The hook is local to your clone — it
lives in `.git/hooks/`, which is not tracked by Git.

## What "validation" actually checks

`scripts\Validate-ResearchOS.py` is the umbrella validator. It runs:

1. Required-file presence (README, CMakeLists, audit doc, architecture
   doc, units header, etc.).
2. README and architecture-doc phrase checks (truth labels, ZIP intake
   rules, milestones).
3. Audit-doc section headings.
4. No tracked secret-like file names and no tracked forbidden paths
   (`_incoming/`, `build/`, `Nautilus/`, archive extensions).
5. Brain corpus structural validation (run only when
   `knowledge/brains/MANIFEST.json` exists).
6. Source-card corpus structural validation (run only when
   `knowledge/papers/INDEX.jsonl` exists).

## CI mirror

The GitHub Actions workflow at `.github/workflows/ci.yml` runs the same
validation gate plus a clean CMake configure/build/test on
`windows-latest`. PRs that pass the daily script locally should pass CI.

## When the workflow drifts

If a script or CI step starts failing on changes you cannot explain:

1. Re-read the relevant ADR in `docs/log/DECISIONS.md`.
2. Check `docs/log/DAILY_LOG.md` for recent notes.
3. Open an issue describing what changed and what you tried.
4. Propose an ADR-NNNN entry for any non-trivial fix.
