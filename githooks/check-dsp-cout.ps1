# Soft RT-safety check: std::cout in source/dsp (exclude tests; allow SCYCLONE_ALLOW_COUT marker)
$ErrorActionPreference = "Continue"
$root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
if (-not (Test-Path (Join-Path $root "source\dsp"))) {
    $root = Split-Path -Parent $PSScriptRoot
}
$hits = rg "std::cout" (Join-Path $root "source\dsp") --type cpp --glob "!*Test*" 2>$null |
    Where-Object {
        if ($_ -match "SCYCLONE_ALLOW_COUT") { return $false }
        $parts = $_ -split ":", 3
        if ($parts.Count -ge 3) {
            return $parts[2] -notmatch "^\s*//"
        }
        return $true
    }
if ($hits) {
    Write-Host "std::cout found in source/dsp (review or remove):"
    $hits | ForEach-Object { Write-Host $_ }
    exit 1
}
Write-Host "No unmarked std::cout in source/dsp"
exit 0
