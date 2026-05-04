// main.rs — Tauri application entry point
// Prevents an extra console window on Windows in release builds.
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

mod commands;
mod database;
mod watcher;

use std::sync::Arc;
use parking_lot::Mutex;
use tauri::Manager;

use database::DbManager;
use watcher::FileWatcher;

/// Shared application state accessible from all Tauri commands.
pub struct AppState {
    /// Thread-safe handle to the SQLite connection manager.
    pub db: Arc<Mutex<DbManager>>,
    /// Thread-safe file watcher instance.
    pub watcher: Arc<Mutex<FileWatcher>>,
}

fn main() {
    // Build the Tauri application.
    tauri::Builder::default()
        // Register all Tauri command handlers (file ops + database ops).
        .invoke_handler(tauri::generate_handler![
            // ── File system commands ──────────────────────────────────────
            commands::file_commands::read_file,
            commands::file_commands::write_file,
            commands::file_commands::delete_file,
            commands::file_commands::create_directory,
            commands::file_commands::list_directory,
            commands::file_commands::rename_entry,
            commands::file_commands::get_file_metadata,
            commands::file_commands::open_folder_dialog,
            commands::file_commands::open_file_dialog,
            commands::file_commands::save_file_dialog,
            // ── Database commands ─────────────────────────────────────────
            commands::db_commands::db_open,
            commands::db_commands::db_execute,
            commands::db_commands::db_query,
            commands::db_commands::db_list_tables,
            commands::db_commands::db_table_schema,
            commands::db_commands::db_list_databases,
            // ── Watcher commands ──────────────────────────────────────────
            commands::file_commands::watch_directory,
            commands::file_commands::unwatch_directory,
        ])
        // Initialise shared state before the window is created.
        .setup(|app| {
            // Initialise the database manager (no file opened yet).
            let db_manager = DbManager::new();
            // Initialise the file watcher (idle until a directory is watched).
            let file_watcher = FileWatcher::new(app.handle().clone());

            // Store state so every command handler can access it.
            app.manage(AppState {
                db: Arc::new(Mutex::new(db_manager)),
                watcher: Arc::new(Mutex::new(file_watcher)),
            });

            Ok(())
        })
        .run(tauri::generate_context!())
        .expect("error while running web-IDE application");
}
