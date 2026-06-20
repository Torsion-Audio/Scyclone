# Configure Scyclone with a specific MSVC toolset from normal PowerShell.
# Other projects can keep using 14.44; only this repo's configure step uses -ToolsetVersion.
#
# Usage:
#   .\cmake\windows\configure.ps1                      # Debug (default preset)
#   .\cmake\windows\configure.ps1 -Preset release      # Release plugin build
#   .\cmake\windows\configure.ps1 -ToolsetVersion 14.44

param(
    [ValidateSet('default', 'release', 'asan')]
    [string]$Preset = 'default',

    [string]$ToolsetVersion = '14.51',

    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ExtraCmakeArgs
)

$ErrorActionPreference = 'Stop'

function Find-VcVars64 {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path $vswhere)) {
        throw 'vswhere.exe not found. Install Visual Studio 2022 with the C++ desktop workload.'
    }

    $vsPath = & $vswhere -latest -products * `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath
    if (-not $vsPath) {
        throw 'Visual Studio 2022 with C++ build tools not found.'
    }

    $vcvars = Join-Path $vsPath 'VC\Auxiliary\Build\vcvars64.bat'
    if (-not (Test-Path $vcvars)) {
        throw "vcvars64.bat not found at $vcvars"
    }

    return @{ VsPath = $vsPath; VcVars = $vcvars }
}

function Test-MsvcToolset {
    param([string]$VsPath, [string]$VersionPrefix)

    $msvcRoot = Join-Path $VsPath 'VC\Tools\MSVC'
    $matches = Get-ChildItem -Path $msvcRoot -Directory -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -like "$VersionPrefix*" }

    return @($matches)
}

$toolchain = Find-VcVars64
$installed = Test-MsvcToolset -VsPath $toolchain.VsPath -VersionPrefix $ToolsetVersion
if ($installed.Count -eq 0) {
    throw @"
MSVC toolset $ToolsetVersion is not installed.

In Visual Studio Installer -> Modify VS 2022 -> Individual components, enable:
  MSVC v143 - VS 2022 C++ x64/x64 build tools (Latest)

That adds 14.51 alongside your existing 14.44 toolset. Your other projects stay on 14.44.
"@
}

$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$userPresets = Join-Path $repoRoot 'CMakeUserPresets.json'
$examplePresets = Join-Path $PSScriptRoot 'CMakeUserPresets.json.example'

if (-not (Test-Path $userPresets) -and (Test-Path $examplePresets)) {
    Copy-Item $examplePresets $userPresets
    Write-Host "Created CMakeUserPresets.json from example."
}

$toolsetInUse = $installed[0].Name
Write-Host "Using MSVC toolset $toolsetInUse for Scyclone ($Preset preset)."

$extraArgs = ($ExtraCmakeArgs -join ' ').Trim()
$cmakeArgs = "--preset $Preset -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl $extraArgs".Trim()

cmd /c "`"$($toolchain.VcVars)`" -vcvars_ver=$ToolsetVersion >nul 2>&1 && set CC=cl&& set CXX=cl&& cd /d `"$repoRoot`" && cmake $cmakeArgs"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
