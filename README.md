# web-IDE

A **production-ready, fully offline web development environment** built with Tauri v2, Rust, Monaco Editor, TypeScript, and SQLite.

Think VS Code + phpMyAdmin + live preview — all running locally without any internet connection.

---

## ✨ Features

| Feature | Details |
|---|---|
| **File System** | Create, read, update, delete files and folders; drag-resizable tree explorer |
| **Code Editor** | Monaco Editor (same engine as VS Code) with syntax highlighting for HTML, CSS, JS, TS, PHP, Python, Rust, SQL and 30+ more |
| **Multi-tab editing** | Open multiple files simultaneously; modified-indicator (`●`); Ctrl+S to save |
| **Live Preview** | Embedded iframe auto-reloads HTML/CSS files on save or disk change |
| **SQLite Database** | Open or create local `.db` / `.sqlite` files; no server required |
| **Query Runner** | Write and execute any SQL (SELECT / INSERT / UPDATE / DELETE / DDL); Ctrl+Enter to run |
| **Table Viewer** | Browse rows with a scrollable data grid; double-click any cell to edit inline |
| **File Watcher** | Rust-backed `notify` watcher emits real-time events; explorer refreshes automatically |
| **Split View** | Draggable vertical divider between editor and preview / database panels |
| **DevTools** | Full Tauri/WebKit developer tools available in debug builds |
| **Offline-first** | Zero CDN dependencies; all Monaco workers bundled by Vite at build time |

---

## 🏗️ Architecture

```
web-IDE/
├── index.html                       # Single-page app shell
├── vite.config.ts                   # Vite + Monaco worker bundling
├── tsconfig.json
├── package.json
│
├── src/                             # TypeScript frontend
│   ├── main.ts                      # App bootstrap & view switcher
│   ├── styles/
│   │   └── main.css                 # Full dark theme (CSS custom properties)
│   ├── services/
│   │   ├── fileService.ts           # Tauri invoke bridge – file operations
│   │   └── dbService.ts             # Tauri invoke bridge – SQLite operations
│   ├── components/
│   │   ├── editor/
│   │   │   ├── Editor.ts            # Monaco EditorPanel (tabs, shortcuts, save)
│   │   │   └── TabManager.ts        # Monaco ITextModel-backed tab state
│   │   ├── explorer/
│   │   │   └── FileExplorer.ts      # File-tree UI (expand/collapse/CRUD)
│   │   ├── preview/
│   │   │   └── Preview.ts           # Blob-URL live preview + auto-reload
│   │   └── database/
│   │       └── DatabasePanel.ts     # Query runner + editable data grid
│   └── layout/
│       └── SplitView.ts             # Resizable split pane + toast system
│
└── src-tauri/                       # Rust backend (Tauri v2)
    ├── tauri.conf.json              # Tauri v2 app configuration
    ├── build.rs
    ├── Cargo.toml
    ├── icons/                       # App icons (PNG/ICO/ICNS)
    └── src/
        ├── main.rs                  # Tauri Builder + AppState registration
        ├── commands/
        │   ├── mod.rs
        │   ├── file_commands.rs     # read_file, write_file, delete_file, list_directory,
        │   │                        # rename_entry, watch_directory, open_folder_dialog, …
        │   └── db_commands.rs       # db_open, db_query, db_execute, db_list_tables,
        │                            # db_table_schema, db_list_databases
        ├── database/
        │   └── mod.rs               # DbManager (rusqlite, WAL mode, schema introspection)
        └── watcher/
            └── mod.rs               # FileWatcher (notify + Tauri event emission)
```

### Data flow

```
Frontend TypeScript
  └─ fileService.ts / dbService.ts
       └─ @tauri-apps/api/core invoke()
            └─ Rust command handler  (src-tauri/src/commands/)
                 ├─ std::fs  (file I/O)
                 ├─ rusqlite (SQLite)
                 └─ notify   (file watcher → Tauri emit("file-changed"))
```

---

## 🚀 Getting Started

### Prerequisites

| Tool | Version | Notes |
|---|---|---|
| [Rust](https://rustup.rs/) | ≥ 1.77 | Install via `rustup` |
| [Node.js](https://nodejs.org/) | ≥ 20 | LTS recommended |
| **Linux only** | – | `libgtk-3-dev`, `libwebkit2gtk-4.1-dev`, `libappindicator3-dev`, `librsvg2-dev` |
| **macOS only** | – | Xcode Command Line Tools |
| **Windows only** | – | WebView2 (included in Windows 11; installer for older versions) |

#### Install Linux dependencies (Ubuntu / Debian)

```bash
sudo apt update
sudo apt install -y \
  libgtk-3-dev \
  libwebkit2gtk-4.1-dev \
  libappindicator3-dev \
  librsvg2-dev \
  libglib2.0-dev
```

### Install & run

```bash
# 1. Install JavaScript dependencies
npm install

# 2. Start the Tauri dev server (opens the app window automatically)
npm run tauri dev
```

### Build for production

```bash
npm run tauri build
# Outputs a native installer in src-tauri/target/release/bundle/
```

---

## ⌨️ Keyboard Shortcuts

| Shortcut | Action |
|---|---|
| `Ctrl/Cmd + O` | Open a project folder |
| `Ctrl/Cmd + S` | Save the active file |
| `Ctrl/Cmd + W` | Close the active tab |
| `Ctrl + Enter` (in DB query editor) | Run SQL query |
| `Tab` (in DB query editor) | Insert 2 spaces |

---

## 🗄️ Database Panel

1. Click **+ Database** in the top toolbar to show the database split pane.
2. Click **📂 Open DB** to pick an existing `.db` / `.sqlite` file, or **✚ New DB** to create one.
3. Tables appear in the left sidebar — click any table to browse its rows.
4. **Inline editing**: double-click any non-PK cell to edit it; press Enter to commit, Escape to cancel.
5. Use the query textarea to run any SQL; `Ctrl+Enter` executes it.

---

## 🔧 Extending the IDE

### Add a new Rust command

1. Write the function in `src-tauri/src/commands/file_commands.rs` or `db_commands.rs` annotated with `#[tauri::command]`.
2. Register it in `src-tauri/src/main.rs` inside `tauri::generate_handler![]`.
3. Call it from TypeScript via `invoke("your_command_name", { ...args })`.

### Add a new language to Monaco

Monaco ships with 80+ language grammars. They are lazy-loaded automatically — just open a file with the right extension.

### Change the colour theme

All colours are defined as CSS custom properties in `src/styles/main.css` under `:root { … }`. Update them there to retheme the entire IDE.

---

## 📦 Tech Stack

| Layer | Technology |
|---|---|
| Desktop shell | [Tauri v2](https://tauri.app/) |
| Backend language | [Rust](https://www.rust-lang.org/) |
| Database | [SQLite](https://sqlite.org/) via [rusqlite](https://github.com/rusqlite/rusqlite) (bundled) |
| File watching | [notify](https://github.com/notify-rs/notify) |
| File dialogs | [rfd](https://github.com/PolyMeilex/rfd) (Rust File Dialog, no extra GTK deps) |
| Frontend language | TypeScript |
| Code editor | [Monaco Editor](https://microsoft.github.io/monaco-editor/) |
| Bundler | [Vite](https://vitejs.dev/) |
| Styling | Plain CSS with custom properties (no framework) |

---

## 📄 License

MIT
