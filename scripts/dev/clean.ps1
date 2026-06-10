#requires -version 5.1
<#
.SYNOPSIS
    Remove the build directory and any generated/cached artifacts.

.EXAMPLE
    .\scripts\dev\clean.ps1
#>
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$RepoRoot = (Resolve-Path "$PSScriptRoot\..\..").Path

$targets = @(
    'build',
    'exports',
    '_incoming\extracted'
)
foreach ($t in $targets) {
    $path = Join-Path $RepoRoot $t
    if (Test-Path $path) {
        Write-Host "Removing $t" -ForegroundColor Yellow
        Remove-Item -Recurse -Force $path
    }
}
Write-Host "Clean OK." -ForegroundColor Green
