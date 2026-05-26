# Repository Workflow

## Source Of Truth

The Git repository is the source of truth. ZIP files are treated as incoming
packages only. If a package contains useful work, it must be extracted, reviewed,
validated, committed, and pushed.

## Daily Loop

```powershell
git status
python scripts/Validate-ResearchOS.py
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Then commit and push:

```powershell
git add -A
git commit -m "Short, factual change description"
git push origin main
```

## Package Intake

Use the local sync script with `-PackagePath` when possible:

```powershell
powershell.exe -ExecutionPolicy Bypass -File .\scripts\local\Sync-OppengrokLocalRepo.ps1 `
  -PackagePath "C:\path\to\package.zip" `
  -RemoteUrl "https://github.com/0thernes/Stress-Test-Agents-Maxxxing.git"
```

Add `-Push` only after validation is expected to pass.

## What Not To Commit

- `_incoming/`
- `build/`
- `exports/`
- package archives
- private keys or credentials
- local Orca/session/daemon state
- nested worktrees such as `Nautilus/`

## Review Discipline

Every meaningful change should answer:

- What research or engineering claim changed?
- What test or validation proves it?
- What truth label applies?
- What docs need to be updated?
- What is the rollback path?
