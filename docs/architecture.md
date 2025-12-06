# Architecture

## Overview
- Embedded C++17 graph store; no external DB.
- Entities: players, teams, matches.
- Edges:
  - `roster`: team -> player
  - `played`: player -> match
- Synergy: player-player aggregates (games, wins) built during ingest.
- Storage: binary files under `store_dir` (`nodes.bin`, `edges.bin`, `synergy.bin`).

## Storage Format (binary)
- Strings are length-prefixed (u32, little endian) followed by bytes.
- `nodes.bin`: count(u32), then [id, type, label]*.
- `edges.bin`: count(u32), then [src, dst, type, weight(double)]*.
- `synergy.bin`: count(u32), then [playerA, playerB, games(int32), wins(int32)]*; one record per undirected pair (a<b).

## Pipeline
1) Fetch: libcurl GET with user-agent + sleep (`FetchConfig`).
2) Parse: Gumbo if available; regex fallback. Extract match IDs, participant summoner IDs, champion keys, win flag, optional team IDs.
3) Ingest:
   - Upsert nodes (player/team/match).
   - Add edges (`played`, `roster`).
   - Update synergy map for all player pairs in a match.
4) Persist: write binaries atomically (truncate+write).
5) Analysis: build adjacency from edges; rank top duos; BFS for reachability/path.

## Threading and Safety
- Current code is single-threaded; ingestion/analysis are in-memory operations on `GraphStore`.
- For concurrency, guard `GraphStore` mutations and consider snapshotting before writes.

## Failure Modes
- Scrape changes: HTML selectors may break; improve parser if op.gg markup shifts.
- Partial writes: binary files are rewritten; keep backups if running in production.
- Corrupt files: add checksums or temp-file + rename strategy if needed.

## Extensibility
- Add weighted shortest-path, centrality, season/league filters.
- Integrate Riot API for stable IDs; enrich team/role parsing.
- Switch to mmap + CSR layout for faster scans; add compression for large stores.

