#requires -version 5.1
<#
.SYNOPSIS
    Regenerate corpora, then run the full Research OS validation chain.

    Order matters: regeneration runs FIRST so that validation checks the
    exact files a subsequent commit would contain. (The 2026-06-12 audit
    flagged the old validate-then-regenerate order as committing
    unvalidated files.)

.EXAMPLE
    .\scripts\dev\audit.ps1
#>
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$RepoRoot = (Resolve-Path "$PSScriptRoot\..\..").Path

# 1. Regenerate corpora from their seeds. Generators are deterministic
#    (dates come from the seed JSON), so this is a no-op unless a seed
#    actually changed.
$brainSeed = Join-Path $RepoRoot 'knowledge\brains\seed_profiles.json'
if (Test-Path $brainSeed) {
    Write-Host "Rebuilding brain corpus..." -ForegroundColor Cyan
    & python (Join-Path $RepoRoot 'scripts\brains\build_brains.py')
    if ($LASTEXITCODE -ne 0) { throw "build_brains failed ($LASTEXITCODE)" }
}

$paperSeed = Join-Path $RepoRoot 'knowledge\papers\seed_sources.json'
if (Test-Path $paperSeed) {
    Write-Host "Rebuilding source-card corpus..." -ForegroundColor Cyan
    & python (Join-Path $RepoRoot 'scripts\research\build_source_cards.py')
    if ($LASTEXITCODE -ne 0) { throw "build_source_cards failed ($LASTEXITCODE)" }
}

# 2. Validate the regenerated state.
Write-Host "Running Research OS validation..." -ForegroundColor Cyan
& python (Join-Path $RepoRoot 'scripts\Validate-ResearchOS.py')
if ($LASTEXITCODE -ne 0) { throw "Validation failed ($LASTEXITCODE)" }

Write-Host "Audit OK." -ForegroundColor Green
