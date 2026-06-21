# Configure Scyclone with a specific MSVC toolset from normal PowerShell.
# Installs/pins the toolset, snapshots vcvars into cmake/windows/generated/msvc-env.cmake,
# then runs cmake --preset. After that, plain cmake --build works without vcvars.
#
# Usage:
#   .\cmake\windows\configure.ps1                      # Debug (default preset)
#   .\cmake\windows\configure.ps1 -Preset release      # Release plugin build
#   .\cmake\windows\configure.ps1 -ToolsetVersion 14.44
#   .\cmake\windows\configure.ps1 -AutoInstallToolset  # install missing toolset without prompting

param(
    [ValidateSet('default', 'release', 'asan')]
    [string]$Preset = 'default',

    [string]$ToolsetVersion = '14.51',

    [switch]$AutoInstallToolset,

    [switch]$NoToolsetInstall,

    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ExtraCmakeArgs
)

$ErrorActionPreference = 'Stop'

$MsvcLatestComponentId = 'Microsoft.VisualStudio.Component.VC.Tools.x86.x64'
$Vs2026CppWorkloadId = 'Microsoft.VisualStudio.Workload.VCTools'
$Vs2026BuildToolsBootstrapperUrl = 'https://aka.ms/vs/stable/vs_buildtools.exe'

function Test-IsAdministrator {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = [Security.Principal.WindowsPrincipal]$identity
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Test-InteractivePrompt {
    return [Environment]::UserInteractive -and -not $AutoInstallToolset -and -not $NoToolsetInstall
}

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
        if ((Test-MsvcToolset -VsPath $inst.installationPath -VersionPrefix $ToolsetVersion).Count -gt 0) {
            $vcvars = Join-Path $inst.installationPath 'VC\Auxiliary\Build\vcvars64.bat'
            if (-not (Test-Path $vcvars)) {
                throw "vcvars64.bat not found at $vcvars"
            }

            return @{
                VsPath = $inst.installationPath
                VcVars = $vcvars
                InstallationVersion = $inst.installationVersion
            }
        }
    }

    $chosen = $ranked[0]
    $vcvars = Join-Path $chosen.installationPath 'VC\Auxiliary\Build\vcvars64.bat'
    if (-not (Test-Path $vcvars)) {
        throw "vcvars64.bat not found at $vcvars"
    }

    return @{
        VsPath = $chosen.installationPath
        VcVars = $vcvars
        InstallationVersion = $chosen.installationVersion
    }
}

function Test-ToolsetNeedsVs2026 {
    param([string]$VersionPrefix)

    # VS 2022 (17.x) ships MSVC through 14.44; 14.51+ comes with VS 2026 (18.x).
    return $VersionPrefix -match '^14\.(5[1-9]|[6-9]\d|\d{3,})'
}

function Get-Vs2026Installations {
    return @(Get-VsInstallations | Where-Object { [version]$_.installationVersion -ge [version]'18.0' })
}

function Get-Vs2026InstallPath {
    $installs = Get-Vs2026Installations
    if ($installs.Count -eq 0) {
        return $null
    }

    $buildTools = $installs | Where-Object { $_.productId -eq 'Microsoft.VisualStudio.Product.BuildTools' } |
        Sort-Object { [version]$_.installationVersion } -Descending
    if ($buildTools.Count -gt 0) {
        return $buildTools[0].installationPath
    }

    return ($installs | Sort-Object { [version]$_.installationVersion } -Descending)[0].installationPath
}

function Get-Vs2026BuildToolsBootstrapper {
    $cacheDir = Join-Path ([Environment]::GetFolderPath('LocalApplicationData')) 'Scyclone\vs-bootstrap'
    New-Item -ItemType Directory -Force -Path $cacheDir | Out-Null
    $bootstrapper = Join-Path $cacheDir 'vs_buildtools.exe'

    if (-not (Test-Path $bootstrapper)) {
        Write-Host 'Downloading Build Tools for Visual Studio 2026 bootstrapper...'
        Invoke-WebRequest -Uri $Vs2026BuildToolsBootstrapperUrl -OutFile $bootstrapper -UseBasicParsing
    }

    return $bootstrapper
}

function Test-VsInstallerSucceeded {
    param([int]$ExitCode)

    # 3010 = ERROR_SUCCESS_REBOOT_REQUIRED (install/modify succeeded; reboot recommended).
    return ($ExitCode -eq 0) -or ($ExitCode -eq 3010)
}

function Assert-VsInstallerSucceeded {
    param(
        [int]$ExitCode,
        [string]$OperationDescription
    )

    if ($ExitCode -eq 3010) {
        Write-Warning "$OperationDescription succeeded; reboot Windows when convenient, then rerun configure if the toolset is still missing."
        return
    }

    if ($ExitCode -ne 0) {
        throw "$OperationDescription failed with exit code $ExitCode."
    }
}

function Invoke-VsBootstrapper {
    param(
        [string]$BootstrapperPath,
        [string[]]$BootstrapperArguments
    )

    $argumentString = Format-VsSetupArguments -SetupArguments $BootstrapperArguments
    Write-Host "Running: `"$BootstrapperPath`" $argumentString"

    if (Test-IsAdministrator) {
        $proc = Start-Process -FilePath $BootstrapperPath -ArgumentList $argumentString -Wait -PassThru
        return $proc.ExitCode
    }

    Write-Host 'Administrator privileges are required. Approve the UAC prompt to continue.'
    $proc = Start-Process -FilePath $BootstrapperPath -ArgumentList $argumentString -Verb RunAs -Wait -PassThru
    return $proc.ExitCode
}

function Install-Vs2026CppBuildTools {
    $existingPath = Get-Vs2026InstallPath
    if ($existingPath) {
        Write-Host "Updating Visual Studio 2026 at:`n  $existingPath"
        $modifyExitCode = Invoke-VsSetup @(
            'modify',
            '--installPath', $existingPath,
            '--add', $Vs2026CppWorkloadId,
            '--add', $MsvcLatestComponentId,
            '--passive',
            '--norestart'
        )
        Wait-VisualStudioInstaller -TimeoutSeconds 7200
        Assert-VsInstallerSucceeded -ExitCode $modifyExitCode -OperationDescription 'Visual Studio Installer modify'
        return
    }

    $bootstrapper = Get-Vs2026BuildToolsBootstrapper
    $workDir = Join-Path ([Environment]::GetFolderPath('LocalApplicationData')) 'Scyclone\vs-bootstrap\run'
    New-Item -ItemType Directory -Force -Path $workDir | Out-Null

    Write-Host 'Installing Build Tools for Visual Studio 2026 (C++ / MSVC Latest)...'
    Write-Host 'This may take several minutes.'

    Push-Location $workDir
    try {
        $installExitCode = Invoke-VsBootstrapper -BootstrapperPath $bootstrapper -BootstrapperArguments @(
            '--passive',
            '--wait',
            '--norestart',
            '--add', $Vs2026CppWorkloadId,
            '--add', $MsvcLatestComponentId
        )
    }
    finally {
        Pop-Location
    }

    Wait-VisualStudioInstaller -TimeoutSeconds 7200
    Assert-VsInstallerSucceeded -ExitCode $installExitCode -OperationDescription 'Visual Studio 2026 Build Tools installation'
}

function Wait-ForMsvcToolset {
    param(
        [string]$VersionPrefix,
        [int]$TimeoutSeconds = 7200
    )

    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    while ((Get-Date) -lt $deadline) {
        foreach ($inst in Get-VsInstallations) {
            $installed = Test-MsvcToolset -VsPath $inst.installationPath -VersionPrefix $VersionPrefix
            if ($installed.Count -gt 0) {
                return $installed
            }
        }

        Start-Sleep -Seconds 5
    }

    return @()
}

function Get-VsSetupPath {
    $setup = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\setup.exe'
    if (-not (Test-Path $setup)) {
        throw 'Visual Studio Installer (setup.exe) not found.'
    }
    return $setup
}

function Test-MsvcToolset {
    param([string]$VsPath, [string]$VersionPrefix)

    $msvcRoot = Join-Path $VsPath 'VC\Tools\MSVC'
    $matches = Get-ChildItem -Path $msvcRoot -Directory -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -like "$VersionPrefix*" }

    return @($matches)
}

function Wait-VisualStudioInstaller {
    param([int]$TimeoutSeconds = 3600)

    $elapsed = 0
    while ($elapsed -lt $TimeoutSeconds) {
        $running = Get-Process -Name 'setup', 'vs_installerservice', 'vs_installershell', 'vs_installer' `
            -ErrorAction SilentlyContinue
        if (-not $running) {
            return
        }

        Start-Sleep -Seconds 3
        $elapsed += 3
    }

    throw 'Timed out waiting for Visual Studio Installer to finish.'
}

function Format-VsSetupArguments {
    param([string[]]$SetupArguments)

    $formatted = foreach ($arg in $SetupArguments) {
        if ($arg -match '\s') { "`"$arg`"" } else { $arg }
    }
    return ($formatted -join ' ')
}

function Invoke-VsSetup {
    param([string[]]$SetupArguments)

    $setup = Get-VsSetupPath
    $argumentString = Format-VsSetupArguments -SetupArguments $SetupArguments
    Write-Host "Running: setup.exe $argumentString"

    if (Test-IsAdministrator) {
        $proc = Start-Process -FilePath $setup -ArgumentList $argumentString -Wait -PassThru
        return $proc.ExitCode
    }

    Write-Host 'Administrator privileges are required. Approve the UAC prompt to continue.'
    $proc = Start-Process -FilePath $setup -ArgumentList $argumentString -Verb RunAs -Wait -PassThru
    return $proc.ExitCode
}

function Install-MsvcToolsetLatest {
    param([string]$InstallPath)

    Write-Host "Updating Visual Studio installer catalog..."
    $updateExitCode = Invoke-VsSetup @(
        'update',
        '--installPath', $InstallPath,
        '--passive',
        '--norestart'
    )
    Wait-VisualStudioInstaller
    if (-not (Test-VsInstallerSucceeded -ExitCode $updateExitCode)) {
        Write-Warning "Visual Studio update exited with code $updateExitCode (continuing)."
    }

    Write-Host "Installing MSVC v143 build tools (Latest)..."
    $modifyExitCode = Invoke-VsSetup @(
        'modify',
        '--installPath', $InstallPath,
        '--add', $MsvcLatestComponentId,
        '--passive',
        '--norestart'
    )
    Wait-VisualStudioInstaller

    Assert-VsInstallerSucceeded -ExitCode $modifyExitCode -OperationDescription 'Visual Studio Installer modify'
}

function Get-MsvcToolsetInstallMessage {
    param(
        [string]$VersionPrefix,
        [bool]$NeedsVs2026 = $false
    )

    if ($NeedsVs2026) {
        return @"
MSVC toolset $VersionPrefix is not installed.

Scyclone links prebuilt static ONNX from anira-project/backends, which requires MSVC $VersionPrefix.
That toolset ships with Visual Studio 2026 (18.x), not Visual Studio 2022 — VS 2022's latest MSVC is 14.44.

Install Build Tools for Visual Studio 2026 with C++ / MSVC $VersionPrefix.
The script can download and install it for you (administrator approval required, several minutes).

Download: https://visualstudio.microsoft.com/downloads/

Toolsets install side-by-side; this script uses $VersionPrefix only for Scyclone builds.
"@
    }

    return @"
MSVC toolset $VersionPrefix is not installed.

Scyclone links prebuilt static ONNX from anira-project/backends, which requires MSVC $VersionPrefix (or newer).

Manual install: Visual Studio Installer -> Modify your VS install -> Individual components ->
  MSVC C++ x64/x86 build tools (Latest)

Toolsets install side-by-side; this script uses $VersionPrefix only for Scyclone builds.
"@
}

function Request-ToolsetInstall {
    param(
        [string]$VersionPrefix,
        [bool]$NeedsVs2026
    )

    if ($AutoInstallToolset) {
        return $true
    }

    if (-not (Test-InteractivePrompt)) {
        throw (Get-MsvcToolsetInstallMessage -VersionPrefix $VersionPrefix -NeedsVs2026 $NeedsVs2026)
    }

    Write-Host (Get-MsvcToolsetInstallMessage -VersionPrefix $VersionPrefix -NeedsVs2026 $NeedsVs2026)
    if ($NeedsVs2026) {
        $prompt = 'Install Build Tools for Visual Studio 2026 (C++/MSVC) now? [Y/n]'
    }
    else {
        $prompt = 'Install/update MSVC build tools (Latest) now? [Y/n]'
    }

    $response = Read-Host $prompt
    if ($response -match '^(n|no)$') {
        throw 'MSVC toolset installation cancelled.'
    }

    return $true
}

function ConvertTo-CMakePath {
    param([string]$Path)
    return ($Path -replace '\\', '/')
}

function Write-MsvcEnvCMake {
    param(
        [string]$OutputPath,
        [string]$ClExe,
        [string]$ToolsetName
    )

    $includes = @($env:INCLUDE -split ';' | Where-Object { $_ -and (Test-Path -LiteralPath $_) } | Select-Object -Unique)
    $libs = @($env:LIB -split ';' | Where-Object { $_ -and (Test-Path -LiteralPath $_) } | Select-Object -Unique)

    if ($includes.Count -eq 0 -or $libs.Count -eq 0) {
        throw 'vcvars did not populate INCLUDE/LIB. Cannot write MSVC environment snapshot.'
    }

    $clForward = ConvertTo-CMakePath $ClExe
    $lines = @(
        '# Generated by cmake/windows/configure.ps1 - do not edit',
        "# Toolset: $ToolsetName",
        "set(SCYCLONE_MSVC_CL `"$clForward`")",
        "set(SCYCLONE_MSVC_CXX `"$clForward`")",
        "set(SCYCLONE_MSVC_TOOLSET `"$ToolsetName`")",
        'set(SCYCLONE_MSVC_INCLUDE_DIRS'
    )

    foreach ($dir in $includes) {
        $lines += "  `"$(ConvertTo-CMakePath $dir)`""
    }

    $lines += ')'
    $lines += 'set(SCYCLONE_MSVC_LIB_DIRS'

    foreach ($dir in $libs) {
        $lines += "  `"$(ConvertTo-CMakePath $dir)`""
    }

    $lines += ')'

    $parent = Split-Path -Parent $OutputPath
    New-Item -ItemType Directory -Force -Path $parent | Out-Null
    Set-Content -Path $OutputPath -Value ($lines -join "`n") -Encoding utf8NoBOM
}

function Ensure-MsvcToolset {
    param(
        [string]$VsPath,
        [string]$VersionPrefix
    )

    $installed = Test-MsvcToolset -VsPath $VsPath -VersionPrefix $VersionPrefix
    if ($installed.Count -gt 0) {
        return $installed
    }

    if ($NoToolsetInstall) {
        throw (Get-MsvcToolsetInstallMessage -VersionPrefix $VersionPrefix -NeedsVs2026 (Test-ToolsetNeedsVs2026 -VersionPrefix $VersionPrefix))
    }

    $needsVs2026 = Test-ToolsetNeedsVs2026 -VersionPrefix $VersionPrefix
    if (-not (Request-ToolsetInstall -VersionPrefix $VersionPrefix -NeedsVs2026 $needsVs2026)) {
        return @()
    }

    if ($needsVs2026) {
        Install-Vs2026CppBuildTools
    }
    else {
        Install-MsvcToolsetLatest -InstallPath $VsPath
    }

    $installed = Wait-ForMsvcToolset -VersionPrefix $VersionPrefix
    if ($installed.Count -eq 0) {
        throw (Get-MsvcToolsetInstallMessage -VersionPrefix $VersionPrefix -NeedsVs2026 $needsVs2026)
    }

    return $installed
}

$toolchain = Find-VcVars64 -ToolsetVersion $ToolsetVersion
$null = Ensure-MsvcToolset -VsPath $toolchain.VsPath -VersionPrefix $ToolsetVersion
$toolchain = Find-VcVars64 -ToolsetVersion $ToolsetVersion
$installed = Test-MsvcToolset -VsPath $toolchain.VsPath -VersionPrefix $ToolsetVersion

$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)

$toolsetInUse = $installed[0].Name
Write-Host "Using MSVC toolset $toolsetInUse for Scyclone ($Preset preset)."

$clExe = Join-Path $installed[0].FullName 'bin\Hostx64\x64\cl.exe'
if (-not (Test-Path $clExe)) {
    throw "MSVC compiler not found at $clExe"
}

$clBin = Split-Path -Parent $clExe

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

Import-VcToolchainEnvironment -VcVars $toolchain.VcVars -VersionPrefix $ToolsetVersion `
    -CompilerBinDir $clBin -CompilerPath $clExe

$envFile = Join-Path $PSScriptRoot 'generated\msvc-env.cmake'
Write-MsvcEnvCMake -OutputPath $envFile -ClExe $clExe -ToolsetName $toolsetInUse
Write-Host "Wrote MSVC environment snapshot to cmake/windows/generated/msvc-env.cmake"

$extraArgs = ($ExtraCmakeArgs -join ' ').Trim()
$cmakeArgs = @('--preset', $Preset)
if ($extraArgs) {
    $cmakeArgs += $extraArgs.Split(' ', [System.StringSplitOptions]::RemoveEmptyEntries)
}

Push-Location $repoRoot
try {
    & cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
finally {
    Pop-Location
}
