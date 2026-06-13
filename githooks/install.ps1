# Point this repository at the version-controlled hooks in githooks/.
$ErrorActionPreference = "Stop"
Set-Location (git rev-parse --show-toplevel)
git config core.hooksPath githooks
Write-Host "Installed git hooks from: $(Get-Location)\githooks"
