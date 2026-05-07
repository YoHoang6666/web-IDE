# web-IDE

A native desktop IDE architecture for offline web development, rebuilt around **C++**, **Qt 6**, **CEF**, and **SQLite**.

## Platform targets

- Primary: Windows desktop installer (`.exe`)
- Future: iPhone / iPad adaptation (`.ipa`)

## Core capabilities

- Professional multi-tab code editor
- Workspace and project explorer
- Embedded local SQLite database tooling
- Live browser preview with Chromium-based rendering
- Inspect element and DevTools integration
- Resizable docked split views
- Real-time file and database synchronization
- Offline-first runtime model

## Repository structure

```text
web-ide/
├─ CMakeLists.txt
├─ cmake/
├─ third_party/
├─ apps/desktop/
├─ src/
│  ├─ core/
│  ├─ shell/
│  ├─ workspace/
│  ├─ editor/
│  ├─ preview/
│  ├─ database/
│  ├─ runtime/
│  ├─ sync/
│  └─ shared/
├─ plugins/
├─ tests/
└─ packaging/
```

## Architecture map

### Shell layer

Qt Widgets, `QMainWindow`, `QDockWidget`, and `QSplitter` provide the native shell, docking, layout presets, menus, and window-level command routing.

### Domain services

- `workspace`: project lifecycle, filesystem access, watcher orchestration
- `editor`: tabbed editing, document buffers, language-service bridge
- `preview`: embedded browser host, session routing, live reload, devtools
- `database`: SQLite connections, query execution, editable result models
- `sync`: change normalization, conflict policy, cross-module propagation

### Infrastructure

- CEF embedding hooks in `src/preview` and `cmake/CEFConfig.cmake`
- SQLite services in `src/database`
- File and DB watchers in `src/workspace` and `src/database`
- Core messaging, commands, logging, and scheduling in `src/core`

### Extensibility

`plugins/sdk` exposes the starter plugin contract and `plugins/CMakeLists.txt` is the module registration point for future packaged plugins.

## Communication flow

1. **Editor save** -> `DocumentManager` -> `FileSystemService` -> `FileWatcherService` -> `SyncCoordinator` -> `LiveReloadController` -> `PreviewPane`
2. **File tree action** -> `WorkspaceManager` / `FileSystemService` -> `EditorTabManager` refresh -> `SyncCoordinator`
3. **SQL execution** -> `QueryEngine` -> `SQLiteConnectionPool` -> `TableModelAdapter` -> `DatabaseWatcherService` -> `SyncCoordinator`
4. **Preview inspect** -> `DevToolsManager` -> `CEFHostWidget`
5. **Long-running work** -> `TaskScheduler`, with events broadcast over `EventBus`

## Build

### Full desktop build

Requires Qt 6 development packages installed and, optionally, CEF assets staged in `third_party/cef`.

```bash
cmake -S /absolute/path/to/web-IDE -B /absolute/path/to/web-IDE/build
cmake --build /absolute/path/to/web-IDE/build
```

### Structure-only validation

Useful in environments without Qt/CEF installed.

```bash
cmake -S /absolute/path/to/web-IDE -B /absolute/path/to/web-IDE/build -DWEBIDE_ENABLE_DESKTOP_APP=OFF
cmake --build /absolute/path/to/web-IDE/build
```
