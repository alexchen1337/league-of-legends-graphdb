# Usage Guide

## Build
```powershell
cmake -S . -B build
cmake --build build --config Release
```

## CLI
```powershell
# ingest + analyze
build\Release\lolgraph_cli data na solo "https://www.op.gg/summoners/na/some-user" playerA playerB
```
- Args: `store_dir region queue url [path_start path_goal]`.
- Omitting `url` skips fetch/ingest and only reads existing binaries to print duos (and optional BFS).
- `store_dir` holds `nodes.bin`, `edges.bin`, `synergy.bin`.

## Rate Limiting
- `FetchConfig` has `sleep_ms` and `user_agent`. Increase `sleep_ms` for production scraping to avoid bans; respect site TOS.

## Parser Expectations
- Looks for `data-game-id` and participant blocks with summoner IDs, champion keys, team win flags.
- Team IDs/roles are heuristic; tighten selectors for better fidelity.
- If Gumbo is present, parsing uses DOM traversal; otherwise falls back to regex.

## Running Tests
```powershell
cmake --build build --config Release --target lolgraph_tests
ctest --test-dir build
```
The sanity test builds an in-memory store, ingests synthetic data, checks synergy and BFS, then saves binaries under `testdata/`.

## Maintenance Tips
- Backup `*.bin` before large ingests.
- If binaries corrupt, delete them and re-ingest from source.
- Improve parsing before long runs; add retries/backoff around curl as needed.

