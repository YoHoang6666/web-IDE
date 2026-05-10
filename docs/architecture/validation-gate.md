# Phase 10 validation gate

Before any subsystem removal or cleanup, validate the following flows end-to-end.

## Required build checks

- Full desktop build (`cmake -S . -B build && cmake --build build`)
- Structure-only validation (`-DWEBIDE_ENABLE_DESKTOP_APP=OFF`)
- Any existing test targets (when enabled)

## Runtime validation checklist

- **Workspace restore**: reopen last workspace on restart, ensure tree + tabs reload.
- **Runtime launch flows**: build/run tasks start, stream output, and exit cleanly.
- **Preview/devtools**: preview loads, devtools attach, and live reload hooks fire.
- **Database tooling**: open SQLite DB, read tables, run queries, edit rows.
- **Terminal lifecycle**: start/cancel/restart commands; verify cleanup on exit.

## Gate rule

No subsystem deletion until every item above passes in the target environment.
