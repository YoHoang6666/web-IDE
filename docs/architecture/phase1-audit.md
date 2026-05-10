# Phase 1 audit: dependency + ownership map

## Dependency map (module-level)

Derived from `CMakeLists.txt` targets.

- **webide_core**: shared headers only.
- **webide_shell** -> webide_core, Qt6::Core/Gui/Widgets/Sql/WebEngineWidgets/WebEngineCore.
- **webide_workspace** -> webide_core, Qt6::Core/Widgets.
- **webide_editor** -> webide_core, webide_workspace, Qt6::Core/Gui/Widgets.
- **webide_preview** -> webide_core, Qt6::Core/Gui/Widgets/Network (and optional CEF glue).
- **webide_database** -> webide_core, Qt6::Core/Sql/Widgets.
- **webide_runtime** -> webide_core, Qt6::Core.
- **webide_sync** -> webide_core, webide_workspace, webide_database, webide_preview, Qt6::Core.

Cross-module dependency direction (avoid cycles):

```
workspace -> core
editor    -> workspace -> core
preview   -> core
database  -> core
runtime   -> core
sync      -> workspace + preview + database -> core
shell     -> editor + preview + database + workspace + runtime -> core
```

## Ownership map (services + UI)

**AppBootstrap** constructs and owns the main services using `std::unique_ptr`:

1. WorkspaceManager
2. FileSystemService
3. FileWatcherService
4. EditorTabManager
5. DocumentManager
6. EditorHost
7. PreviewPane
8. PreviewSessionManager
9. LiveReloadController
10. DevToolsManager
11. DatabaseManager
12. SQLiteConnectionPool
13. QueryEngine
14. DatabaseWatcherService
15. SyncCoordinator
16. MainWindow

**MainWindow** owns the UI tree:

- `MainWindow` (QMainWindow)
  - central widget: `EditorAreaWidget`
  - dock widgets: Explorer / Preview / Database / Terminal / Network
  - each dock owns its respective panel widget

**Widget ownership** follows Qt parent-child rules (widget created with `this` parent).

## QObject parent ownership

- Most services (`WorkspaceManager`, `FileSystemService`, `FileWatcherService`, etc.) are QObjects but intentionally **unparented**; lifecycle is managed by `AppBootstrap` via `std::unique_ptr` to avoid circular QObject trees prior to AppContext/ServiceContainer.
- UI widgets use standard QObject parent ownership (`MainWindow` -> docks -> widgets).

This split keeps service lifetimes explicit and prevents parent cycles during migration.

## Startup / shutdown order

**Startup:**

1. `main.cpp` creates `QApplication`.
2. `AppBootstrap::run` constructs services in dependency order.
3. `wireCoreServices` connects service dependencies.
4. `MainWindow` is constructed and shown.
5. Qt event loop runs.

**Shutdown:**

1. Qt event loop exits (window closed).
2. `MainWindow` and UI tree are destroyed.
3. `AppBootstrap` destructs and releases service `unique_ptr`s in reverse construction order.

## Compile dependency audit

Compile-level dependencies are intentionally shallow:

- **Shell** is the only layer that directly touches most domains.
- **Sync** is the only module that composes Workspace + Database + Preview.
- **Runtime** has no inbound dependencies today, keeping process/runtime growth isolated.

This keeps ownership refactors safe by avoiding compile-time cycles between domain libraries.
