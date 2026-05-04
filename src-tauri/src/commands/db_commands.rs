// commands/db_commands.rs
// Tauri commands that expose SQLite database operations to the frontend.
// The database state is kept in `AppState::db` (a `DbManager` behind a Mutex).

use serde::{Deserialize, Serialize};
use tauri::State;

use crate::database::{QueryResult, TableInfo};
use crate::AppState;

// ─────────────────────────────────────────────────────────────────────────────
// Helper
// ─────────────────────────────────────────────────────────────────────────────

fn err_str<E: std::fmt::Display>(e: E) -> String {
    e.to_string()
}

// ─────────────────────────────────────────────────────────────────────────────
// DTOs
// ─────────────────────────────────────────────────────────────────────────────

/// Information about an opened/available database.
#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct DatabaseInfo {
    /// Absolute path to the `.db` file.
    pub path: String,
    /// Friendly display name (basename without extension).
    pub name: String,
}

// ─────────────────────────────────────────────────────────────────────────────
// Commands
// ─────────────────────────────────────────────────────────────────────────────

/// Open (or create) an SQLite database at `path`.
/// After this call all other `db_*` commands operate on this database.
#[tauri::command]
pub async fn db_open(
    path: String,
    state: State<'_, AppState>,
) -> Result<DatabaseInfo, String> {
    let mut db = state.db.lock();
    db.open(&path).map_err(err_str)?;

    let name = std::path::Path::new(&path)
        .file_stem()
        .and_then(|s| s.to_str())
        .unwrap_or("database")
        .to_string();

    Ok(DatabaseInfo { path, name })
}

/// Execute a SQL statement that does **not** return rows
/// (INSERT, UPDATE, DELETE, CREATE, DROP, …).
/// Returns the number of rows affected.
#[tauri::command]
pub async fn db_execute(
    sql: String,
    state: State<'_, AppState>,
) -> Result<usize, String> {
    let db = state.db.lock();
    db.execute(&sql).map_err(err_str)
}

/// Execute a SQL SELECT and return all matching rows as JSON-serialisable
/// `QueryResult` (column names + row data).
#[tauri::command]
pub async fn db_query(
    sql: String,
    state: State<'_, AppState>,
) -> Result<QueryResult, String> {
    let db = state.db.lock();
    db.query(&sql).map_err(err_str)
}

/// Return the names of all user-created tables in the open database.
#[tauri::command]
pub async fn db_list_tables(
    state: State<'_, AppState>,
) -> Result<Vec<String>, String> {
    let db = state.db.lock();
    db.list_tables().map_err(err_str)
}

/// Return the column definitions for `table_name`.
#[tauri::command]
pub async fn db_table_schema(
    table_name: String,
    state: State<'_, AppState>,
) -> Result<Vec<TableInfo>, String> {
    let db = state.db.lock();
    db.table_schema(&table_name).map_err(err_str)
}

/// Return a list of all `.db` / `.sqlite` files found in `directory`.
/// Useful for showing a "recent databases" picker in the UI.
#[tauri::command]
pub async fn db_list_databases(directory: String) -> Result<Vec<DatabaseInfo>, String> {
    let mut databases = Vec::new();
    let entries = std::fs::read_dir(&directory).map_err(err_str)?;

    for entry in entries.flatten() {
        let path = entry.path();
        if let Some(ext) = path.extension().and_then(|e| e.to_str()) {
            if matches!(ext.to_lowercase().as_str(), "db" | "sqlite" | "sqlite3") {
                let name = path
                    .file_stem()
                    .and_then(|s| s.to_str())
                    .unwrap_or("database")
                    .to_string();
                databases.push(DatabaseInfo {
                    path: path.to_string_lossy().to_string(),
                    name,
                });
            }
        }
    }

    databases.sort_by(|a, b| a.name.cmp(&b.name));
    Ok(databases)
}
