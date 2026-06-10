#requires -version 5.1
<#
.SYNOPSIS
    Run the full Research OS validation chain.

    Wraps scripts\Validate-ResearchOS.py, which itself fans out to the
    brain and source-card validators when those corpora exist.

.EXAMPLE
    .\scripts\dev\audit.ps1
#>
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$RepoRoot = (Resolve-Path "$PSScriptRoot\..\..").Path

Write-Host "Running Research OS validation..." -ForegroundColor Cyan
& python (Join-Path $RepoRoot 'scripts\Validate-ResearchOS.py')
if ($LASTEXITCODE -ne 0) { throw "Validation failed ($LASTEXITCODE)" }

# Regenerate brain and source-card corpora if their seeds exist. Cheap and
# keeps generated artifacts in sync with the seed JSON.
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

Write-Host "Audit OK." -ForegroundColor Green
