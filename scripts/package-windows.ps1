param(
    [switch]$Clean,
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$releaseBuild = Join-Path $root "build-release"
$dist = Join-Path $root "dist\MalloyWriter-win64"
$exe = Join-Path $releaseBuild "MalloyWriter.exe"

function Resolve-Tool {
    param([string[]]$Candidates)
    foreach ($candidate in $Candidates) {
        if (Test-Path $candidate) {
            return (Resolve-Path $candidate).Path
        }
        $command = Get-Command $candidate -ErrorAction SilentlyContinue
        if ($command) {
            return $command.Source
        }
    }
    throw "Could not find tool. Tried: $($Candidates -join ', ')"
}

if ($Clean -and (Test-Path (Join-Path $root "dist"))) {
    Remove-Item -Recurse -Force (Join-Path $root "dist")
}

if (-not $SkipBuild) {
    & (Join-Path $root "build.ps1") -Configuration Release -RunTests
}

if (-not (Test-Path $exe)) {
    throw "Missing release executable: $exe"
}

New-Item -ItemType Directory -Force -Path $dist | Out-Null
Copy-Item -Force $exe $dist
Copy-Item -Force (Join-Path $root "README.md") $dist
Copy-Item -Force (Join-Path $root "LICENSE") $dist

$deployQt = Resolve-Tool @(
    "C:\msys64\mingw64\bin\windeployqt6.exe",
    "C:\msys64\mingw64\bin\windeployqt-qt6.exe",
    "windeployqt6",
    "windeployqt"
)

& $deployQt --release --compiler-runtime (Join-Path $dist "MalloyWriter.exe")

$archive = Join-Path $root "dist\MalloyWriter-win64.zip"
if (Test-Path $archive) {
    Remove-Item -Force $archive
}
Compress-Archive -Path (Join-Path $dist "*") -DestinationPath $archive
Write-Host "Packaged $archive"
