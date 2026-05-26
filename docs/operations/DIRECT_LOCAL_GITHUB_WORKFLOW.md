# Direct Local GitHub Workflow

This repository is the source of truth. Do not use a downloaded ZIP as the final workflow. Incoming repo packages are only staging inputs; the live local Git repo owns validation, commits, rebases, and pushes.

## Expected Location

The intended repo location is:

```powershell
Z:\Orca\Workspaces\Stress Test Agents Maxxxing\oppengrok-blackhole-sim
```

If this repo is currently one directory higher, move or clone it into that folder before treating the path as final.

## One-Time Remote Check

The repo must point to GitHub, not a sandbox-local path:

```powershell
git remote -v
git remote set-url origin "https://github.com/OWNER/REPO.git"
```

If `origin` does not exist:

```powershell
git remote add origin "https://github.com/OWNER/REPO.git"
```

Replace `OWNER/REPO` with the actual GitHub repository.

## Normal Sync

Put downloaded OPPENGROK repo packages in Downloads or another source folder, then run:

```powershell
cd "Z:\Orca\Workspaces\Stress Test Agents Maxxxing\oppengrok-blackhole-sim"

powershell.exe -ExecutionPolicy Bypass -File .\scripts\local\Sync-OppengrokLocalRepo.ps1 `
  -Workspace "Z:\Orca\Workspaces\Stress Test Agents Maxxxing" `
  -SourceDirectory "$env:USERPROFILE\Downloads" `
  -RemoteUrl "https://github.com/OWNER/REPO.git" `
  -Push
```

The script:

- archives the selected package into `_incoming/packages/`
- extracts it into `_incoming/extracted/`
- selects the newest matching repo package from `-SourceDirectory`, or uses `-PackagePath` when supplied
- creates a backup branch before syncing
- copies files into the live repo without copying `.git`
- runs `scripts/Validate-ResearchOS.py` when present
- runs CMake configure/build/CTest when `CMakeLists.txt` is present
- runs `oppengrok_blackhole` as a smoke test when the executable is produced
- checks for obvious secret-like file names visible to Git
- commits the resulting tree
- fetches, rebases onto `origin/main`, and pushes when `-Push` is supplied

## Destructive Mirror Sync

Use `-Prune` only when the selected incoming package should become the exact live tree:

```powershell
powershell.exe -ExecutionPolicy Bypass -File .\scripts\local\Sync-OppengrokLocalRepo.ps1 `
  -Workspace "Z:\Orca\Workspaces\Stress Test Agents Maxxxing" `
  -SourceDirectory "$env:USERPROFILE\Downloads" `
  -RemoteUrl "https://github.com/OWNER/REPO.git" `
  -Prune `
  -Push
```

`-Prune` removes live repo files that are absent from the selected package, except for `.git` and `_incoming`. The script creates a backup branch first, but review the branch and status before pushing.

## Local-Only Validation

To validate and commit locally without pushing:

```powershell
powershell.exe -ExecutionPolicy Bypass -File .\scripts\local\Sync-OppengrokLocalRepo.ps1 `
  -SourceDirectory "$env:USERPROFILE\Downloads"
```

If Downloads contains unrelated archives, pass the package explicitly:

```powershell
powershell.exe -ExecutionPolicy Bypass -File .\scripts\local\Sync-OppengrokLocalRepo.ps1 `
  -PackagePath "$env:USERPROFILE\Downloads\oppengrok-blackhole-sim-v5-research-os.zip" `
  -RemoteUrl "https://github.com/OWNER/REPO.git" `
  -Push
```

To skip CMake temporarily while checking the rest of the workflow:

```powershell
powershell.exe -ExecutionPolicy Bypass -File .\scripts\local\Sync-OppengrokLocalRepo.ps1 `
  -SourceDirectory "$env:USERPROFILE\Downloads" `
  -SkipBuild
```

## Recovery

Every run creates a branch named like:

```text
backup/local-sync/YYYYMMDD-HHMMSS
```

To inspect it:

```powershell
git branch --list "backup/local-sync/*"
git diff backup/local-sync/YYYYMMDD-HHMMSS..HEAD
```

To return to that backup manually, create a new branch from it rather than rewriting history:

```powershell
git switch -c recovery/from-local-sync backup/local-sync/YYYYMMDD-HHMMSS
```
