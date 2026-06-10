#requires -version 5.1
<#
.SYNOPSIS
    Install the Validate-ResearchOS pre-commit hook into .git/hooks/.

    Re-running this script is idempotent; it overwrites the hook file.

.EXAMPLE
    .\scripts\dev\Install-PreCommitHook.ps1
#>
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$RepoRoot = (Resolve-Path "$PSScriptRoot\..\..").Path
$HookDir = Join-Path $RepoRoot '.git\hooks'
if (-not (Test-Path $HookDir)) {
    throw "$HookDir does not exist. Is this a git working tree?"
}

$hook = Join-Path $HookDir 'pre-commit'
$body = @'
#!/bin/sh
# Pre-commit hook installed by scripts/dev/Install-PreCommitHook.ps1
# Runs the Research OS validation gate. Aborts the commit on failure.
set -e
python scripts/Validate-ResearchOS.py
'@
Set-Content -Path $hook -Value $body -Encoding ASCII -NoNewline
Write-Host "Installed pre-commit hook at $hook" -ForegroundColor Green
