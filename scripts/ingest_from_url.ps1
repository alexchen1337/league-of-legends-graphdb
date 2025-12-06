# Requires: built lolgraph_cli in build\Release
param(
    [Parameter(Mandatory = $true)][string]$StoreDir,
    [Parameter(Mandatory = $true)][string]$Region,
    [Parameter(Mandatory = $true)][string]$Queue,
    [Parameter(Mandatory = $true)][string]$Url
)

$exe = "build\Release\lolgraph_cli.exe"
if (-not (Test-Path $exe)) {
    Write-Error "Executable not found at $exe. Build first."
    exit 1
}

& $exe $StoreDir $Region $Queue $Url

