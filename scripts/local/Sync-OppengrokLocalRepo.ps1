[CmdletBinding()]
param(
    [string]$Workspace,
    [string]$SourceDirectory = (Join-Path $env:USERPROFILE "Downloads"),
    [string]$RemoteUrl,
    [string]$MainBranch = "main",
    [switch]$Prune,
    [switch]$Push,
    [switch]$SkipBuild
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$RunStamp = Get-Date -Format "yyyyMMdd-HHmmss"
$RepoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot "..\..")).ProviderPath
if (-not $Workspace) {
    $Workspace = Split-Path -Parent $RepoRoot
}

function Write-Step {
    param([string]$Message)
    Write-Host ""
    Write-Host "==> $Message" -ForegroundColor Cyan
}

function Write-Note {
    param([string]$Message)
    Write-Host "    $Message"
}

function Invoke-External {
    param(
        [Parameter(Mandatory = $true)][string]$File,
        [string[]]$Arguments = @(),
        [string]$WorkingDirectory = $RepoRoot,
        [switch]$AllowFailure
    )

    Write-Host ">> $File $($Arguments -join ' ')" -ForegroundColor DarkGray
    Push-Location -LiteralPath $WorkingDirectory
    try {
        $output = & $File @Arguments 2>&1
        $exitCode = $LASTEXITCODE
    }
    finally {
        Pop-Location
    }

    foreach ($line in $output) {
        Write-Host $line
    }

    if (-not $AllowFailure -and $exitCode -ne 0) {
        throw "$File failed with exit code $exitCode"
    }

    return $exitCode
}

function Invoke-Git {
    param(
        [Parameter(Mandatory = $true)][string[]]$Arguments,
        [switch]$AllowFailure
    )

    $args = @("-C", $RepoRoot) + $Arguments
    return Invoke-External -File "git" -Arguments $args -AllowFailure:$AllowFailure
}

function Get-GitOutput {
    param([Parameter(Mandatory = $true)][string[]]$Arguments)

    $args = @("-C", $RepoRoot) + $Arguments
    $output = & git @args
    if ($LASTEXITCODE -ne 0) {
        throw "git $($Arguments -join ' ') failed with exit code $LASTEXITCODE"
    }

    return @($output)
}

function Get-FullPath {
    param([Parameter(Mandatory = $true)][string]$Path)
    return [System.IO.Path]::GetFullPath($Path)
}

function Assert-UnderRepo {
    param([Parameter(Mandatory = $true)][string]$Path)

    $root = (Get-FullPath $RepoRoot).TrimEnd([System.IO.Path]::DirectorySeparatorChar, [System.IO.Path]::AltDirectorySeparatorChar)
    $full = Get-FullPath $Path
    if (-not $full.StartsWith($root + [System.IO.Path]::DirectorySeparatorChar, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to touch path outside repo root: $full"
    }
}

function Get-CandidatePackages {
    param([Parameter(Mandatory = $true)][string]$Root)

    if (-not (Test-Path -LiteralPath $Root)) {
        Write-Note "Source directory does not exist: $Root"
        return @()
    }

    $archivePattern = '\.(zip|tar|tgz|tar\.gz)$'
    return @(Get-ChildItem -LiteralPath $Root -File -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -match $archivePattern } |
        Sort-Object LastWriteTimeUtc -Descending)
}

function Copy-PackageArchive {
    param(
        [Parameter(Mandatory = $true)][System.IO.FileInfo]$Package,
        [Parameter(Mandatory = $true)][string]$IncomingPackages
    )

    $target = Join-Path $IncomingPackages $Package.Name
    if (Test-Path -LiteralPath $target) {
        $name = [System.IO.Path]::GetFileNameWithoutExtension($Package.Name)
        $ext = [System.IO.Path]::GetExtension($Package.Name)
        if ($Package.Name -match '\.tar\.gz$') {
            $name = $Package.Name.Substring(0, $Package.Name.Length - 7)
            $ext = ".tar.gz"
        }
        $target = Join-Path $IncomingPackages ("{0}-{1}{2}" -f $name, $RunStamp, $ext)
    }

    Copy-Item -LiteralPath $Package.FullName -Destination $target -Force
    return (Resolve-Path -LiteralPath $target).ProviderPath
}

function Expand-Package {
    param(
        [Parameter(Mandatory = $true)][string]$ArchivePath,
        [Parameter(Mandatory = $true)][string]$ExtractRoot
    )

    $safeName = [System.IO.Path]::GetFileName($ArchivePath) -replace '[^A-Za-z0-9._-]', '_'
    $safeName = $safeName -replace '\.tar\.gz$', '' -replace '\.zip$', '' -replace '\.tgz$', '' -replace '\.tar$', ''
    $target = Join-Path $ExtractRoot ("{0}-{1}" -f $safeName, $RunStamp)
    New-Item -ItemType Directory -Path $target -Force | Out-Null

    if ($ArchivePath -match '\.zip$') {
        Expand-Archive -LiteralPath $ArchivePath -DestinationPath $target -Force
    }
    elseif ($ArchivePath -match '\.(tar|tgz|tar\.gz)$') {
        Invoke-External -File "tar" -Arguments @("-xf", $ArchivePath, "-C", $target)
    }
    else {
        throw "Unsupported package format: $ArchivePath"
    }

    return (Resolve-Path -LiteralPath $target).ProviderPath
}

function Test-RepoPackageRoot {
    param([Parameter(Mandatory = $true)][string]$Path)

    $hasCMake = Test-Path -LiteralPath (Join-Path $Path "CMakeLists.txt")
    $hasResearchValidator = Test-Path -LiteralPath (Join-Path $Path "scripts\Validate-ResearchOS.py")
    $hasReadme = Test-Path -LiteralPath (Join-Path $Path "README.md")
    $hasSource = (Test-Path -LiteralPath (Join-Path $Path "src")) -or (Test-Path -LiteralPath (Join-Path $Path "include"))
    return $hasCMake -or $hasResearchValidator -or ($hasReadme -and $hasSource)
}

function Find-RepoPackageRoot {
    param([Parameter(Mandatory = $true)][string]$ExtractedPath)

    $candidates = New-Object System.Collections.Generic.List[string]
    $candidates.Add($ExtractedPath)
    Get-ChildItem -LiteralPath $ExtractedPath -Directory -Recurse -Force |
        Where-Object { $_.FullName -notmatch '[\\/]\.git([\\/]|$)' } |
        ForEach-Object { $candidates.Add($_.FullName) }

    foreach ($candidate in $candidates) {
        if (Test-RepoPackageRoot -Path $candidate) {
            return (Resolve-Path -LiteralPath $candidate).ProviderPath
        }
    }

    throw "No valid repo package root found under: $ExtractedPath"
}

function New-BackupBranch {
    $head = (Get-GitOutput -Arguments @("rev-parse", "--verify", "HEAD"))[0]
    $branch = "backup/local-sync/$RunStamp"
    Invoke-Git -Arguments @("branch", $branch, $head) | Out-Null
    return $branch
}

function Remove-IfTypeMismatch {
    param(
        [Parameter(Mandatory = $true)][System.IO.FileSystemInfo]$SourceEntry,
        [Parameter(Mandatory = $true)][string]$DestinationPath
    )

    if (-not (Test-Path -LiteralPath $DestinationPath)) {
        return
    }

    $destination = Get-Item -LiteralPath $DestinationPath -Force
    if ($SourceEntry.PSIsContainer -ne $destination.PSIsContainer) {
        Assert-UnderRepo -Path $DestinationPath
        Remove-Item -LiteralPath $DestinationPath -Recurse -Force
    }
}

function Sync-RepoContents {
    param([Parameter(Mandatory = $true)][string]$SourceRoot)

    $protectedTopLevel = @(".git", "_incoming")

    if ($Prune) {
        Write-Step "Pruning live tree to match selected package"
        Get-ChildItem -LiteralPath $RepoRoot -Force |
            Where-Object { $protectedTopLevel -notcontains $_.Name } |
            ForEach-Object {
                $sourcePeer = Join-Path $SourceRoot $_.Name
                if (-not (Test-Path -LiteralPath $sourcePeer)) {
                    Assert-UnderRepo -Path $_.FullName
                    Write-Note "Removing $($_.FullName)"
                    Remove-Item -LiteralPath $_.FullName -Recurse -Force
                }
            }
    }

    Write-Step "Copying selected package into live repo"
    Get-ChildItem -LiteralPath $SourceRoot -Force |
        Where-Object { $protectedTopLevel -notcontains $_.Name } |
        ForEach-Object {
            $dest = Join-Path $RepoRoot $_.Name
            Remove-IfTypeMismatch -SourceEntry $_ -DestinationPath $dest
            Copy-Item -LiteralPath $_.FullName -Destination $RepoRoot -Recurse -Force
        }
}

function Invoke-ResearchValidation {
    $validator = Join-Path $RepoRoot "scripts\Validate-ResearchOS.py"
    if (-not (Test-Path -LiteralPath $validator)) {
        Write-Note "No Research OS validator found; skipping."
        return
    }

    Write-Step "Running Research OS validation"
    $python = Get-Command python -ErrorAction SilentlyContinue
    if ($python) {
        Invoke-External -File "python" -Arguments @($validator)
        return
    }

    $py = Get-Command py -ErrorAction SilentlyContinue
    if ($py) {
        Invoke-External -File "py" -Arguments @("-3", $validator)
        return
    }

    throw "Research OS validator exists, but neither python nor py is available."
}

function Invoke-CMakeValidation {
    if ($SkipBuild) {
        Write-Note "SkipBuild supplied; skipping CMake configure/build/test."
        return
    }

    $cmakeLists = Join-Path $RepoRoot "CMakeLists.txt"
    if (-not (Test-Path -LiteralPath $cmakeLists)) {
        Write-Note "No CMakeLists.txt found; skipping CMake build."
        return
    }

    if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
        throw "CMakeLists.txt exists, but cmake is not available on PATH."
    }

    $buildDir = Join-Path $RepoRoot "build\local-sync-release"
    Write-Step "Configuring CMake"
    Invoke-External -File "cmake" -Arguments @("-S", $RepoRoot, "-B", $buildDir, "-DCMAKE_BUILD_TYPE=Release")

    Write-Step "Building"
    Invoke-External -File "cmake" -Arguments @("--build", $buildDir, "--config", "Release")

    Write-Step "Running CTest"
    Invoke-External -File "ctest" -Arguments @("--test-dir", $buildDir, "-C", "Release", "--output-on-failure")

    $exe = Get-ChildItem -LiteralPath $buildDir -Recurse -File -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -match '^oppengrok_blackhole(\.exe)?$' } |
        Select-Object -First 1

    if ($exe) {
        Write-Step "Running executable smoke check"
        Invoke-External -File $exe.FullName -Arguments @()
    }
}

function Test-ObviousSecretLikeFiles {
    Write-Step "Checking obvious secret-like file names"
    $paths = Get-GitOutput -Arguments @("ls-files", "--cached", "--others", "--exclude-standard")
    $secretPattern = '(?i)(^|[\\/])(\.env(\.|$)|.*(secret|secrets|credential|credentials|token|api[_-]?key|apikey|private[_-]?key|password|passwd).*)$|(?i)(\.pem|\.pfx|\.p12|\.key|\.keystore|\.jks)$|(?i)(^|[\\/])id_(rsa|dsa|ecdsa|ed25519)'
    $hits = @($paths | Where-Object { $_ -match $secretPattern })

    if ($hits.Count -gt 0) {
        $list = $hits -join [Environment]::NewLine
        throw "Refusing to continue because secret-like files are visible to git:$([Environment]::NewLine)$list"
    }

    Write-Note "No obvious secret-like file names visible to git."
}

function Commit-Changes {
    $status = Get-GitOutput -Arguments @("status", "--short")
    if ($status.Count -eq 0) {
        Write-Note "No changes to commit."
        return $false
    }

    Write-Step "Committing changes"
    Invoke-Git -Arguments @("add", "-A") | Out-Null
    Invoke-Git -Arguments @("commit", "-m", "Sync local OPPENGROK repo package") | Out-Null
    return $true
}

function Set-OriginRemote {
    param([string]$Url)

    if ([string]::IsNullOrWhiteSpace($Url)) {
        return
    }

    Write-Step "Configuring origin remote"
    $remotes = Get-GitOutput -Arguments @("remote")
    if ($remotes -contains "origin") {
        Invoke-Git -Arguments @("remote", "set-url", "origin", $Url) | Out-Null
    }
    else {
        Invoke-Git -Arguments @("remote", "add", "origin", $Url) | Out-Null
    }
}

function Rebase-OriginMain {
    if (-not (Get-GitOutput -Arguments @("remote") | Where-Object { $_ -eq "origin" })) {
        Write-Note "No origin remote configured; skipping fetch/rebase."
        return
    }

    Write-Step "Fetching origin"
    Invoke-Git -Arguments @("fetch", "origin") | Out-Null

    $lsRemoteExit = Invoke-Git -Arguments @("ls-remote", "--exit-code", "--heads", "origin", $MainBranch) -AllowFailure
    if ($lsRemoteExit -eq 0) {
        Write-Step "Rebasing onto origin/$MainBranch"
        Invoke-Git -Arguments @("rebase", "origin/$MainBranch") | Out-Null
    }
    else {
        Write-Note "origin/$MainBranch was not found; skipping rebase."
    }
}

function Push-OriginMain {
    if (-not $Push) {
        Write-Note "Push not requested. Use -Push when you are ready to publish."
        return
    }

    Write-Step "Pushing to origin/$MainBranch"
    Invoke-Git -Arguments @("push", "-u", "origin", $MainBranch) | Out-Null
}

Write-Step "Preparing direct local repo workflow"
Write-Note "Repo root: $RepoRoot"
Write-Note "Workspace: $Workspace"
Write-Note "Source directory: $SourceDirectory"

if (-not (Test-Path -LiteralPath (Join-Path $RepoRoot ".git"))) {
    throw "This script must live inside a real Git repository: $RepoRoot"
}

$incomingRoot = Join-Path $RepoRoot "_incoming"
$incomingPackages = Join-Path $incomingRoot "packages"
$extractRoot = Join-Path $incomingRoot "extracted"
New-Item -ItemType Directory -Path $incomingPackages, $extractRoot -Force | Out-Null

$backupBranch = New-BackupBranch
Write-Note "Backup branch created: $backupBranch"

$packages = Get-CandidatePackages -Root $SourceDirectory
if ($packages.Count -gt 0) {
    $package = $packages[0]
    Write-Step "Selected newest repo package"
    Write-Note "$($package.FullName)"

    $archivedPackage = Copy-PackageArchive -Package $package -IncomingPackages $incomingPackages
    Write-Note "Archived package: $archivedPackage"

    $extractedPath = Expand-Package -ArchivePath $archivedPackage -ExtractRoot $extractRoot
    $selectedRoot = Find-RepoPackageRoot -ExtractedPath $extractedPath
    Write-Note "Selected package root: $selectedRoot"
    Sync-RepoContents -SourceRoot $selectedRoot
}
else {
    Write-Note "No supported packages found in source directory; validating current repo tree."
}

Set-OriginRemote -Url $RemoteUrl
Invoke-ResearchValidation
Invoke-CMakeValidation
Test-ObviousSecretLikeFiles
$committed = Commit-Changes

if ($Push) {
    Rebase-OriginMain
    Push-OriginMain
}
elseif ($committed) {
    Write-Note "Committed locally. Review with: git -C `"$RepoRoot`" log -1 --stat"
}

Write-Step "Direct local repo workflow complete"
