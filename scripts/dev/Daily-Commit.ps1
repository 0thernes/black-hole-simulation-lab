#requires -version 5.1
<#
.SYNOPSIS
    Validate, build, test, commit, and push the day's work.

.DESCRIPTION
    The daily forward-motion script. Defaults are safe: it will refuse to
    push if validation or tests fail, and will not commit unless the user
    supplies a message.

.PARAMETER Message
    Commit message. Required when there are staged changes.

.PARAMETER Push
    Push to origin after a successful commit.

.PARAMETER SkipBuild
    Skip the CMake build step (useful when only docs changed).

.PARAMETER SkipTests
    Skip CTest (useful when only docs changed).

.EXAMPLE
    .\scripts\dev\Daily-Commit.ps1 -Message "Document Eshkol Stage 1" -Push
#>
[CmdletBinding()]
param(
    [string]$Message,
    [switch]$Push,
    [switch]$SkipBuild,
    [switch]$SkipTests
)

$ErrorActionPreference = 'Stop'
$RepoRoot = (Resolve-Path "$PSScriptRoot\..\..").Path
Push-Location $RepoRoot
try {
    Write-Host "Daily-Commit: starting in $RepoRoot" -ForegroundColor Cyan

    # 1. Validation gate (always run; cheap).
    & (Join-Path $RepoRoot 'scripts\dev\audit.ps1')

    # 2. Build.
    if (-not $SkipBuild) {
        & (Join-Path $RepoRoot 'scripts\dev\build.ps1') -Configuration Release
    } else {
        Write-Host "Skipping build (-SkipBuild)" -ForegroundColor Yellow
    }

    # 3. Tests.
    if (-not $SkipTests) {
        & (Join-Path $RepoRoot 'scripts\dev\test.ps1') -Configuration Release
    } else {
        Write-Host "Skipping tests (-SkipTests)" -ForegroundColor Yellow
    }

    # 4. Status snapshot.
    Write-Host "git status:" -ForegroundColor Cyan
    & git status --short

    $hasChanges = ((& git status --porcelain) | Measure-Object).Count -gt 0
    if (-not $hasChanges) {
        Write-Host "No changes to commit. Skipping commit and push." -ForegroundColor Yellow
        return
    }

    if (-not $Message) {
        Write-Host "Pass -Message to commit." -ForegroundColor Yellow
        return
    }

    & git add -A
    & git commit -m $Message
    if ($LASTEXITCODE -ne 0) { throw "git commit failed ($LASTEXITCODE)" }

    if ($Push) {
        & git push origin main
        if ($LASTEXITCODE -ne 0) { throw "git push failed ($LASTEXITCODE)" }
        Write-Host "Pushed to origin/main." -ForegroundColor Green
    } else {
        Write-Host "Committed locally. Pass -Push to push to origin." -ForegroundColor Green
    }
} finally {
    Pop-Location
}
