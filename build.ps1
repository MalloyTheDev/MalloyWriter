param(
    [switch]$RunTests,
    [switch]$Clean,
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug"
)

$ErrorActionPreference = "Stop"

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

$buildDir = Join-Path $PSScriptRoot ($(if ($Configuration -eq "Release") { "build-release" } else { "build" }))
$qtCmake = Resolve-Tool @("C:\msys64\mingw64\bin\qt-cmake.bat", "qt-cmake.bat", "qt-cmake", "cmake")
$ninja = Resolve-Tool @("C:\ProgramData\chocolatey\bin\ninja.exe", "C:\msys64\mingw64\bin\ninja.exe", "ninja")
$gcc = Resolve-Tool @("C:\msys64\mingw64\bin\gcc.exe", "gcc")
$gxx = Resolve-Tool @("C:\msys64\mingw64\bin\g++.exe", "g++")

if ($Clean -and (Test-Path $buildDir)) {
    Remove-Item -Recurse -Force $buildDir
}

& $qtCmake -S $PSScriptRoot -B $buildDir -G Ninja `
    "-DCMAKE_BUILD_TYPE=$Configuration" `
    "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON" `
    "-DCMAKE_MAKE_PROGRAM=$ninja" `
    "-DCMAKE_C_COMPILER=$gcc" `
    "-DCMAKE_CXX_COMPILER=$gxx"

cmake --build $buildDir

if ($RunTests) {
    ctest --test-dir $buildDir --output-on-failure
}
