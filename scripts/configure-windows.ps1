# Configure Scyclone with MSVC from a normal PowerShell session.
# Requires Visual Studio 2022 with the C++ workload installed.

$ErrorActionPreference = "Stop"

$vcvars = "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path $vcvars)) {
    $vcvars = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
}
if (-not (Test-Path $vcvars)) {
    Write-Error "vcvars64.bat not found. Install VS 2022 with the C++ desktop workload."
}

$repoRoot = Split-Path -Parent $PSScriptRoot
$userPresets = Join-Path $repoRoot "CMakeUserPresets.json"
$examplePresets = Join-Path $repoRoot "CMakeUserPresets.json.example"

if (-not (Test-Path $userPresets)) {
    Copy-Item $examplePresets $userPresets
    Write-Host "Created CMakeUserPresets.json from example."
}

$configureArgs = $args -join " "
if ([string]::IsNullOrWhiteSpace($configureArgs)) {
    $configureArgs = "--preset default-msvc"
}

cmd /c "`"$vcvars`" >nul 2>&1 && set CC=cl&& set CXX=cl&& cd /d `"$repoRoot`" && cmake $configureArgs"
