# external/

Third-party integrations live here. Each subdirectory follows the pattern:

```text
external/<vendor>/<name>/
  README.md            Upstream URL, commit SHA pin, license, adapter notes
  patches/             Optional local patches applied before build
  CMakeLists.txt       Adapter-level CMake glue (when applicable)
```

## Rules

1. **No direct upstream code commits.** External code is fetched via
   CMake `FetchContent` or referenced as a git submodule. The local
   `external/<vendor>/<name>/` directory holds the adapter, not a copy of
   the upstream tree.
2. **Pin everything by commit SHA.** Tracking `main` or `master` is a
   review-blocker. The upstream commit and license must be recorded in
   the adapter's `README.md`.
3. **Each integration requires an ADR.** Document the scope, the wrap
   surface, the build-system impact, the license review, and the rollback
   plan. ADR file under `docs/log/DECISIONS.md`.
4. **Adapter-only wrappers.** Public headers live under
   `include/blackhole_ds/external/<name>.hpp`. The kernel never includes
   upstream headers directly; it goes through the adapter.
5. **Integrations are opt-in.** Each integration is gated behind a CMake
   option (e.g., `BHDS_ENABLE_LIBIRREP`). Default is OFF. The kernel must
   build, test, and run without any external integration enabled.

## Roster (planned)

See `docs/integrations/TSOTCHKE_ECOSYSTEM.md` for the full ecosystem map
and integration sequencing.

Current plan:

- `external/tsotchke/libirrep/` — Tier B, target Q2.
- `external/tsotchke/eshkol/` — Tier C, target Q3.
- `external/tsotchke/quantum_geometric_tensor/` — Tier C, target Q3.
- `external/tsotchke/moonlab/` — Tier D, target Year 2+.

## Templates

When you add the first real integration, copy the layout from one of the
ADR-driven examples below and follow the rules above:

```text
external/tsotchke/libirrep/
  README.md
  patches/
  CMakeLists.txt
```

The adapter `CMakeLists.txt` uses `FetchContent_Declare` and
`FetchContent_MakeAvailable`, pinned by commit SHA.
