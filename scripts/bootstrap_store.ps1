# Initializes an empty store directory
param(
    [Parameter(Mandatory = $true)][string]$StoreDir
)

if (-not (Test-Path $StoreDir)) {
    New-Item -ItemType Directory -Path $StoreDir | Out-Null
}

"Initialized store at $StoreDir"

