# Build Scyclone with the same MSVC toolset used by configure.ps1.
#
# Usage:
#   .\cmake\windows\build.ps1                        # full Debug build
#   .\cmake\windows\build.ps1 -BuildPreset test      # Test target only
#   .\cmake\windows\build.ps1 -BuildPreset release   # full Release build
#   .\cmake\windows\build.ps1 -Target Scyclone_VST3 Scyclone_Standalone

param(
    [ValidateSet('default', 'release', 'test', 'asan')]
    [string]$BuildPreset = 'default',

    [string]$ToolsetVersion = '14.51',

    [string[]]$Target = @()
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

    return $vcvars
}

$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$vcvars = Find-VcVars64

$buildArgs = @('--build', '--preset', $BuildPreset, '--parallel')
if ($Target.Count -gt 0) {
    $buildArgs += '--target'
    $buildArgs += $Target
}

$buildCommand = 'cmake ' + ($buildArgs -join ' ')
Write-Host "Building with MSVC $ToolsetVersion ($BuildPreset preset)..."

cmd /c "`"$vcvars`" -vcvars_ver=$ToolsetVersion >nul 2>&1 && cd /d `"$repoRoot`" && $buildCommand"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
