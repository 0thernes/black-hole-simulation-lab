#requires -version 5.1
<#
.SYNOPSIS
    Run CTest against the build tree.

.PARAMETER Configuration
    Release (default) or Debug.

.EXAMPLE
    .\scripts\dev\test.ps1
#>
[CmdletBinding()]
param(
    [ValidateSet('Release', 'Debug')]
    [string]$Configuration = 'Release'
)

$ErrorActionPreference = 'Stop'
$RepoRoot = (Resolve-Path "$PSScriptRoot\..\..").Path
$BuildDir = Join-Path $RepoRoot 'build'

if (-not (Test-Path $BuildDir)) {
    throw "Build directory missing. Run scripts\dev\build.ps1 first."
}

$ctest = 'C:\Program Files\CMake\bin\ctest.exe'
if (-not (Test-Path $ctest)) {
    $cmd = Get-Command ctest -ErrorAction SilentlyContinue
    if ($cmd) { $ctest = $cmd.Source } else { throw 'ctest not found' }
}

Write-Host "Running CTest..." -ForegroundColor Cyan
& $ctest --test-dir $BuildDir -C $Configuration --output-on-failure
if ($LASTEXITCODE -ne 0) { throw "CTest failed ($LASTEXITCODE)" }
Write-Host "Tests OK." -ForegroundColor Green
