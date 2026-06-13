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
# Force LF line endings. If this .ps1 is checked out with CRLF (the Windows
# default), the here-string would otherwise carry CRLF into the generated
# sh hook, producing an invalid "#!/bin/sh\r" shebang that Git's hook
# runner rejects. Writing raw bytes bypasses Set-Content's platform
# newline handling entirely. (Audit 2026-06-12, finding F-022.)
$body = $body -replace "`r`n", "`n"
[System.IO.File]::WriteAllText($hook, $body, [System.Text.UTF8Encoding]::new($false))
Write-Host "Installed pre-commit hook at $hook (LF-normalized)" -ForegroundColor Green
