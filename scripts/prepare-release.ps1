#Requires -Version 5.1
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$Version
)

$ErrorActionPreference = 'Stop'

if ($Version -notmatch '^\d+\.\d+\.\d+$') {
    Write-Error "Version must be semver X.Y.Z (got '$Version')"
}

$Root = Split-Path -Parent $PSScriptRoot
$CMakeFile = Join-Path $Root 'CMakeLists.txt'

$Content = Get-Content $CMakeFile -Raw
if ($Content -notmatch 'project\s*\(\s*Scyclone\s+VERSION\s+(?<ver>[0-9.]+)') {
    Write-Error "Could not read project(Scyclone VERSION ...) from CMakeLists.txt"
}

$CmakeVersion = $Matches['ver']

Write-Host "CMake VERSION:  $CmakeVersion"
Write-Host "Target release: $Version"
Write-Host "Tag to push:    v$Version"
Write-Host

if ($CmakeVersion -ne $Version) {
    Write-Error "Mismatch: update CMakeLists.txt to project(Scyclone VERSION $Version) before tagging."
}

Write-Host "Version check passed."
Write-Host
Write-Host "Next steps:"
Write-Host "  git tag v$Version"
Write-Host "  git push origin v$Version"
Write-Host
Write-Host "CI will validate, build signed zips, and publish a GitHub Release."
