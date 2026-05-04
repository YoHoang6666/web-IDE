// commands/file_commands.rs
// All Tauri commands that deal with the local file system.
// Each public function decorated with #[tauri::command] becomes callable
// from the TypeScript front-end via `invoke("command_name", { ...args })`.

use std::fs;
use std::path::{Path, PathBuf};
use serde::{Deserialize, Serialize};
use tauri::State;

use crate::AppState;
use crate::watcher::WatchResult;

// ─────────────────────────────────────────────────────────────────────────────
// Data transfer objects
// ─────────────────────────────────────────────────────────────────────────────

/// Metadata for a single file-system entry (file or directory).
#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct FileEntry {
    /// Absolute path on disk.
    pub path: String,
    /// File/folder name without the parent path.
    pub name: String,
    /// `true` when the entry is a directory.
    pub is_dir: bool,
    /// File size in bytes (0 for directories).
    pub size: u64,
    /// Last-modified timestamp as an ISO-8601 string.
    pub modified: String,
    /// File extension (empty string for directories or files without extension).
    pub extension: String,
}

// Helper: convert any Display-able error into a String so Tauri can serialise it.
fn err_str<E: std::fmt::Display>(e: E) -> String {
    e.to_string()
}

// ─────────────────────────────────────────────────────────────────────────────
// File commands
// ─────────────────────────────────────────────────────────────────────────────

/// Read the UTF-8 content of a file at `path`.
#[tauri::command]
pub async fn read_file(path: String) -> Result<String, String> {
    fs::read_to_string(&path).map_err(err_str)
}

/// Write `content` to a file at `path`, creating parent directories if needed.
#[tauri::command]
pub async fn write_file(path: String, content: String) -> Result<(), String> {
    // Ensure parent directory exists before writing.
    if let Some(parent) = Path::new(&path).parent() {
        fs::create_dir_all(parent).map_err(err_str)?;
    }
    fs::write(&path, content).map_err(err_str)
}

/// Delete a file or an empty directory at `path`.
#[tauri::command]
pub async fn delete_file(path: String) -> Result<(), String> {
    let p = Path::new(&path);
    if p.is_dir() {
        // Remove directory and all its contents.
        fs::remove_dir_all(p).map_err(err_str)
    } else {
        fs::remove_file(p).map_err(err_str)
    }
}

/// Create a directory (and all missing parent directories) at `path`.
#[tauri::command]
pub async fn create_directory(path: String) -> Result<(), String> {
    fs::create_dir_all(&path).map_err(err_str)
}

/// List all immediate children of `path` and return their metadata.
#[tauri::command]
pub async fn list_directory(path: String) -> Result<Vec<FileEntry>, String> {
    let entries = fs::read_dir(&path).map_err(err_str)?;
    let mut result: Vec<FileEntry> = Vec::new();

    for entry in entries {
        let entry = entry.map_err(err_str)?;
        let meta = entry.metadata().map_err(err_str)?;
        let entry_path = entry.path();

        // Format the modified time as an ISO-8601 string.
        let modified = meta
            .modified()
            .ok()
            .and_then(|t| {
                t.duration_since(std::time::UNIX_EPOCH).ok().map(|d| {
                    let secs = d.as_secs();
                    // Minimal ISO-8601 UTC representation without extra dependencies.
                    let dt = chrono::DateTime::<chrono::Utc>::from_timestamp(secs as i64, 0)
                        .unwrap_or_default();
                    dt.to_rfc3339()
                })
            })
            .unwrap_or_default();

        let extension = entry_path
            .extension()
            .and_then(|e| e.to_str())
            .unwrap_or("")
            .to_string();

        result.push(FileEntry {
            path: entry_path.to_string_lossy().to_string(),
            name: entry.file_name().to_string_lossy().to_string(),
            is_dir: meta.is_dir(),
            size: meta.len(),
            modified,
            extension,
        });
    }

    // Directories first, then files — both sorted alphabetically.
    result.sort_by(|a, b| {
        b.is_dir
            .cmp(&a.is_dir)
            .then_with(|| a.name.to_lowercase().cmp(&b.name.to_lowercase()))
    });

    Ok(result)
}

/// Rename (or move) an entry from `from` to `to`.
#[tauri::command]
pub async fn rename_entry(from: String, to: String) -> Result<(), String> {
    fs::rename(&from, &to).map_err(err_str)
}

/// Return metadata for a single file or directory.
#[tauri::command]
pub async fn get_file_metadata(path: String) -> Result<FileEntry, String> {
    let p = PathBuf::from(&path);
    let meta = fs::metadata(&p).map_err(err_str)?;

    let modified = meta
        .modified()
        .ok()
        .and_then(|t| {
            t.duration_since(std::time::UNIX_EPOCH).ok().map(|d| {
                let secs = d.as_secs();
                let dt = chrono::DateTime::<chrono::Utc>::from_timestamp(secs as i64, 0)
                    .unwrap_or_default();
                dt.to_rfc3339()
            })
        })
        .unwrap_or_default();

    let extension = p
        .extension()
        .and_then(|e| e.to_str())
        .unwrap_or("")
        .to_string();

    Ok(FileEntry {
        name: p
            .file_name()
            .map(|n| n.to_string_lossy().to_string())
            .unwrap_or_default(),
        path,
        is_dir: meta.is_dir(),
        size: meta.len(),
        modified,
        extension,
    })
}

// ─────────────────────────────────────────────────────────────────────────────
// Native dialog helpers
// ─────────────────────────────────────────────────────────────────────────────

/// Open a native folder-picker dialog and return the chosen path (if any).
#[tauri::command]
pub async fn open_folder_dialog() -> Result<Option<String>, String> {
    let path = rfd::FileDialog::new().pick_folder();
    Ok(path.map(|p| p.to_string_lossy().to_string()))
}

/// Open a native file-picker dialog and return the chosen path (if any).
#[tauri::command]
pub async fn open_file_dialog() -> Result<Option<String>, String> {
    let path = rfd::FileDialog::new()
        .add_filter("SQLite Database", &["db", "sqlite", "sqlite3"])
        .pick_file();
    Ok(path.map(|p| p.to_string_lossy().to_string()))
}

/// Open a native save-file dialog and return the chosen path (if any).
#[tauri::command]
pub async fn save_file_dialog(default_name: String) -> Result<Option<String>, String> {
    let path = rfd::FileDialog::new()
        .add_filter("SQLite Database", &["db"])
        .set_file_name(&default_name)
        .save_file();
    Ok(path.map(|p| p.to_string_lossy().to_string()))
}

// ─────────────────────────────────────────────────────────────────────────────
// File watcher commands
// ─────────────────────────────────────────────────────────────────────────────

/// Start watching a directory for changes; emits `file-changed` events to the
/// front-end whenever a file is created, modified, or deleted.
#[tauri::command]
pub async fn watch_directory(
    path: String,
    state: State<'_, AppState>,
) -> Result<WatchResult, String> {
    let mut watcher = state.watcher.lock();
    watcher.watch(&path).map_err(err_str)
}

/// Stop watching a previously registered directory.
#[tauri::command]
pub async fn unwatch_directory(
    path: String,
    state: State<'_, AppState>,
) -> Result<(), String> {
    let mut watcher = state.watcher.lock();
    watcher.unwatch(&path).map_err(err_str)
}
