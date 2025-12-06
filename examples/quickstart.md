# Quickstart

## Ingest and analyze
```powershell
# build once
cmake -S . -B build
cmake --build build --config Release

# ingest from an op.gg profile and show duos + path
build\Release\lolgraph_cli data na solo "https://www.op.gg/summoners/na/some-user" faker t1
```

## Re-run analysis without fetching
```powershell
build\Release\lolgraph_cli data
```

## Inspect generated binaries
- `data/nodes.bin`: players/teams/matches
- `data/edges.bin`: roster and played edges
- `data/synergy.bin`: player-player aggregates

## Sample synthetic HTML generation
See `scripts/data_generation/synthetic_html.py` to create a minimal HTML that the parser can ingest for offline testing.

