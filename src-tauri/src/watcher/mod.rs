// watcher/mod.rs
// Cross-platform file-system watcher built on top of the `notify` crate.
// When a watched directory changes the watcher emits a `file-changed` Tauri
// event so the frontend can refresh the file-explorer tree and reload the
// live-preview pane without polling.

use anyhow::{anyhow, Result};
use notify::{Config, Event, EventKind, RecommendedWatcher, RecursiveMode, Watcher};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::path::Path;
use std::sync::mpsc::{channel, Receiver};
use std::thread;
use tauri::{AppHandle, Emitter};

// ─────────────────────────────────────────────────────────────────────────────
// Public types
// ─────────────────────────────────────────────────────────────────────────────

/// Confirmation returned to the frontend after a `watch_directory` call.
#[derive(Debug, Serialize, Deserialize)]
pub struct WatchResult {
    /// The directory that is now being watched.
    pub path: String,
    /// Informational message.
    pub message: String,
}

/// Payload emitted with each `file-changed` event.
#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct FileChangeEvent {
    /// Absolute path of the affected file / directory.
    pub path: String,
    /// One of: "create" | "modify" | "delete" | "rename" | "other"
    pub kind: String,
}

// ─────────────────────────────────────────────────────────────────────────────
// FileWatcher
// ─────────────────────────────────────────────────────────────────────────────

/// Wraps a `notify` watcher and keeps track of all currently watched paths.
pub struct FileWatcher {
    /// The underlying watcher (kept alive for as long as we want to watch).
    inner: Option<RecommendedWatcher>,
    /// Map from watched path → placeholder (for future per-path removal).
    watched: HashMap<String, ()>,
    /// Tauri app handle used to emit events to the frontend.
    app: AppHandle,
}

impl FileWatcher {
    /// Construct a new, idle watcher.
    pub fn new(app: AppHandle) -> Self {
        Self {
            inner: None,
            watched: HashMap::new(),
            app,
        }
    }

    /// Start watching `path` recursively.
    /// Emits `file-changed` events on the Tauri event bus.
    pub fn watch(&mut self, path: &str) -> Result<WatchResult> {
        // Lazily create the underlying watcher on the first call.
        if self.inner.is_none() {
            let app_handle = self.app.clone();
            let (tx, rx) = channel::<notify::Result<Event>>();

            let watcher = RecommendedWatcher::new(tx, Config::default())
                .map_err(|e| anyhow!("Failed to create file watcher: {}", e))?;

            self.inner = Some(watcher);

            // Spin up a background thread that forwards notify events to Tauri.
            thread::spawn(move || {
                forward_events(rx, app_handle);
            });
        }

        // Register the requested path with the watcher.
        if let Some(ref mut w) = self.inner {
            w.watch(Path::new(path), RecursiveMode::Recursive)
                .map_err(|e| anyhow!("Failed to watch {}: {}", path, e))?;
        }

        self.watched.insert(path.to_string(), ());

        Ok(WatchResult {
            path: path.to_string(),
            message: format!("Now watching: {}", path),
        })
    }

    /// Stop watching `path`.
    pub fn unwatch(&mut self, path: &str) -> Result<()> {
        if let Some(ref mut w) = self.inner {
            w.unwatch(Path::new(path))
                .map_err(|e| anyhow!("Failed to unwatch {}: {}", path, e))?;
        }
        self.watched.remove(path);
        Ok(())
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Background thread
// ─────────────────────────────────────────────────────────────────────────────

/// Runs in a dedicated thread; receives raw `notify` events and re-emits them
/// as typed `file-changed` Tauri window events.
fn forward_events(rx: Receiver<notify::Result<Event>>, app: AppHandle) {
    for result in rx {
        match result {
            Ok(event) => {
                let kind_str = match event.kind {
                    EventKind::Create(_) => "create",
                    EventKind::Modify(_) => "modify",
                    EventKind::Remove(_) => "delete",
                    EventKind::Access(_) => continue, // ignore pure access events
                    _ => "other",
                };

                for path in &event.paths {
                    let payload = FileChangeEvent {
                        path: path.to_string_lossy().to_string(),
                        kind: kind_str.to_string(),
                    };

                    // Emit to all windows; the frontend filters by path prefix.
                    let _ = app.emit("file-changed", &payload);
                }
            }
            Err(e) => {
                eprintln!("[watcher] error: {:?}", e);
            }
        }
    }
}
