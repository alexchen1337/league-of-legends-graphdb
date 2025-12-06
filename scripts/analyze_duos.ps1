# Prints top duos and optional path between two nodes
param(
    [Parameter(Mandatory = $true)][string]$StoreDir,
    [string]$Start = "",
    [string]$Goal = ""
)

$exe = "build\Release\lolgraph_cli.exe"
if (-not (Test-Path $exe)) {
    Write-Error "Executable not found at $exe. Build first."
    exit 1
}

if ($Start -and $Goal) {
    & $exe $StoreDir "na" "solo" "" $Start $Goal
} else {
    & $exe $StoreDir "na" "solo"
}

