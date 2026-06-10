#requires -version 5.1
<#
.SYNOPSIS
    Configure and build BlackHoleDS with CMake + MinGW.

.PARAMETER Configuration
    Release (default) or Debug.

.PARAMETER Clean
    Wipe the build directory before configuring.

.EXAMPLE
    .\scripts\dev\build.ps1
    .\scripts\dev\build.ps1 -Configuration Debug -Clean
#>
[CmdletBinding()]
param(
    [ValidateSet('Release', 'Debug')]
    [string]$Configuration = 'Release',

    [switch]$Clean
)

$ErrorActionPreference = 'Stop'
$RepoRoot = (Resolve-Path "$PSScriptRoot\..\..").Path
$BuildDir = Join-Path $RepoRoot 'build'

function Find-Tool([string]$Name, [string[]]$Candidates) {
    foreach ($p in $Candidates) {
        if (Test-Path $p) { return $p }
    }
    $cmd = Get-Command $Name -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    throw "Cannot find $Name. Tried: $($Candidates -join ', ')"
}

$cmake = Find-Tool 'cmake' @(
    'C:\Program Files\CMake\bin\cmake.exe'
)
$gxx = Find-Tool 'g++' @(
    'C:\Program Files\CodeBlocks\MinGW\bin\g++.exe'
)

if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host "Cleaning $BuildDir" -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}

Write-Host "Configuring ($Configuration)..." -ForegroundColor Cyan
& $cmake -S $RepoRoot -B $BuildDir `
    -DCMAKE_BUILD_TYPE=$Configuration `
    -G 'MinGW Makefiles' `
    -DCMAKE_CXX_COMPILER="$gxx"
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed ($LASTEXITCODE)" }

Write-Host "Building..." -ForegroundColor Cyan
& $cmake --build $BuildDir
if ($LASTEXITCODE -ne 0) { throw "Build failed ($LASTEXITCODE)" }

Write-Host "Build OK." -ForegroundColor Green
