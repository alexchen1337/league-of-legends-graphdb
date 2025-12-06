# Tests

## Build and run
```powershell
cmake --build build --config Release --target lolgraph_tests
ctest --test-dir build
```

## What is covered
- Sanity: in-memory GraphStore ingest of synthetic match, synergy calc, BFS path, binary save.

## Fixtures
- Generate offline HTML with `scripts/data_generation/synthetic_html.py` and ingest via CLI to smoke test parsing without hitting the network.

