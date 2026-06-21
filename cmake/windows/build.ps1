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

function Get-VsInstallations {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path $vswhere)) {
        throw 'vswhere.exe not found. Install Visual Studio with the C++ desktop workload.'
    }

    $json = & $vswhere -all -prerelease -products * `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -format json 2>$null
    if (-not $json) {
        return @()
    }

    return @($json | ConvertFrom-Json)
}

function Find-VcVars64 {
    param([string]$ToolsetVersion = '14.51')

    $installs = Get-VsInstallations
    if ($installs.Count -eq 0) {
        throw 'Visual Studio with C++ build tools not found.'
    }

    $ranked = $installs | Sort-Object { [version]$_.installationVersion } -Descending
    foreach ($inst in $ranked) {
        $msvcRoot = Join-Path $inst.installationPath 'VC\Tools\MSVC'
        $toolset = Get-ChildItem -Path $msvcRoot -Directory -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -like "$ToolsetVersion*" } |
            Select-Object -First 1
        if ($toolset) {
            $vcvars = Join-Path $inst.installationPath 'VC\Auxiliary\Build\vcvars64.bat'
            if (-not (Test-Path $vcvars)) {
                throw "vcvars64.bat not found at $vcvars"
            }

            return @{
                VcVars = $vcvars
                ToolsetDir = $toolset
            }
        }
    }

    throw "MSVC toolset $ToolsetVersion is not installed. Run .\cmake\windows\configure.ps1 first."
}

function Import-VcToolchainEnvironment {
    param(
        [string]$VcVars,
        [string]$VersionPrefix,
        [string]$CompilerBinDir,
        [string]$CompilerPath
    )

    cmd /c "`"$VcVars`" -vcvars_ver=$VersionPrefix >nul 2>&1 && set" | ForEach-Object {
        if ($_ -match '^([^=]+)=(.*)$') {
            Set-Item -Path "Env:\$($matches[1])" -Value $matches[2] -Force
        }
    }

    $env:CC = $CompilerPath
    $env:CXX = $CompilerPath
    $env:PATH = "$CompilerBinDir;$env:PATH"
}

$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$toolchain = Find-VcVars64 -ToolsetVersion $ToolsetVersion
$clExe = Join-Path $toolchain.ToolsetDir.FullName 'bin\Hostx64\x64\cl.exe'
$clBin = Split-Path -Parent $clExe

Import-VcToolchainEnvironment -VcVars $toolchain.VcVars -VersionPrefix $ToolsetVersion `
    -CompilerBinDir $clBin -CompilerPath $clExe

Write-Host "Building with MSVC $($toolchain.ToolsetDir.Name) ($BuildPreset preset)..."

$buildArgs = @('--build', '--preset', $BuildPreset, '--parallel')
if ($Target.Count -gt 0) {
    $buildArgs += '--target'
    $buildArgs += $Target
}

Push-Location $repoRoot
try {
    & cmake @buildArgs
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
finally {
    Pop-Location
}
