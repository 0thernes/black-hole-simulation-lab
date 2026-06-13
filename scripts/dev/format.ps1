#requires -version 5.1
<#
.SYNOPSIS
    Apply or check clang-format on the C++ tree.

.DESCRIPTION
    Uses the repo's .clang-format. CI enforces the same style with
    clang-format 19.1.7; install a matching version locally
    (`pip install clang-format==19.1.7`) to avoid surprises.

.PARAMETER Check
    Dry-run only: fail if any file is not already formatted (what CI does).
    Without this switch, files are formatted in place.

.EXAMPLE
    .\scripts\dev\format.ps1            # format in place
    .\scripts\dev\format.ps1 -Check     # verify only
#>
[CmdletBinding()]
param(
    [switch]$Check
)

$ErrorActionPreference = 'Stop'
$RepoRoot = (Resolve-Path "$PSScriptRoot\..\..").Path

function Resolve-ClangFormat {
    $candidates = @(
        'clang-format',
        'C:\Program Files\CodeBlocks\MinGW\bin\clang-format.exe',
        'C:\msys64\clang64\bin\clang-format.exe',
        'C:\Program Files\LLVM\bin\clang-format.exe'
    )
    foreach ($c in $candidates) {
        $cmd = Get-Command $c -ErrorAction SilentlyContinue
        if ($cmd) { return $cmd.Source }
        if (Test-Path $c) { return $c }
    }
    throw "clang-format not found. Install with: pip install clang-format==19.1.7"
}

$cf = Resolve-ClangFormat
$styleArg = "-style=file:$RepoRoot\.clang-format"

$files = Get-ChildItem -Path "$RepoRoot\include", "$RepoRoot\src", "$RepoRoot\tests" `
    -Recurse -Include *.hpp, *.cpp

if ($Check) {
    & $cf --dry-run --Werror $styleArg ($files.FullName)
    if ($LASTEXITCODE -ne 0) { throw "Formatting check failed ($LASTEXITCODE)" }
    Write-Host "Formatting OK ($($files.Count) files)." -ForegroundColor Green
} else {
    foreach ($f in $files) { & $cf -i $styleArg $f.FullName }
    Write-Host "Formatted $($files.Count) files." -ForegroundColor Green
}
