/**
 * fileService.ts
 * Bridge between the TypeScript frontend and the Rust file-system commands
 * exposed by Tauri.  All methods call `invoke` with the matching Rust command
 * name; errors are surfaced as rejected Promises so callers can use try/catch.
 */

import { invoke } from "@tauri-apps/api/core";
import { listen, Event as TauriEvent } from "@tauri-apps/api/event";

// ─────────────────────────────────────────────────────────────────────────────
// Shared types (must mirror the Rust DTOs in file_commands.rs)
// ─────────────────────────────────────────────────────────────────────────────

export interface FileEntry {
  /** Absolute path on disk. */
  path: string;
  /** File / folder name without parent path. */
  name: string;
  /** `true` when the entry is a directory. */
  is_dir: boolean;
  /** File size in bytes (0 for directories). */
  size: number;
  /** Last-modified timestamp as ISO-8601 string. */
  modified: string;
  /** File extension (e.g. "ts", "html") or empty string. */
  extension: string;
}

export interface FileChangeEvent {
  /** Absolute path of the changed file. */
  path: string;
  /** "create" | "modify" | "delete" | "rename" | "other" */
  kind: string;
}

export type FileChangeCallback = (event: FileChangeEvent) => void;

// ─────────────────────────────────────────────────────────────────────────────
// FileService
// ─────────────────────────────────────────────────────────────────────────────

class FileService {
  /**
   * Read and return the UTF-8 contents of a file.
   */
  async readFile(path: string): Promise<string> {
    return invoke<string>("read_file", { path });
  }

  /**
   * Write `content` to a file, creating intermediate directories as needed.
   */
  async writeFile(path: string, content: string): Promise<void> {
    return invoke<void>("write_file", { path, content });
  }

  /**
   * Delete a file or directory (recursively if directory).
   */
  async deleteFile(path: string): Promise<void> {
    return invoke<void>("delete_file", { path });
  }

  /**
   * Create a directory (and any missing parent directories).
   */
  async createDirectory(path: string): Promise<void> {
    return invoke<void>("create_directory", { path });
  }

  /**
   * List the immediate children of a directory, sorted directories-first.
   */
  async listDirectory(path: string): Promise<FileEntry[]> {
    return invoke<FileEntry[]>("list_directory", { path });
  }

  /**
   * Rename or move an entry.
   */
  async renameEntry(from: string, to: string): Promise<void> {
    return invoke<void>("rename_entry", { from, to });
  }

  /**
   * Return metadata for a single file or directory.
   */
  async getFileMetadata(path: string): Promise<FileEntry> {
    return invoke<FileEntry>("get_file_metadata", { path });
  }

  /**
   * Open a native folder-picker dialog; resolves with the selected path
   * or `null` if the user cancelled.
   */
  async openFolderDialog(): Promise<string | null> {
    return invoke<string | null>("open_folder_dialog");
  }

  /**
   * Start watching `path` for changes.
   * Calls `callback` each time a change event arrives from the watcher.
   * Returns an unsubscribe function.
   */
  async watchDirectory(
    path: string,
    callback: FileChangeCallback
  ): Promise<() => void> {
    // Ask the Rust watcher to start monitoring the path.
    await invoke("watch_directory", { path });

    // Subscribe to the Tauri event that the watcher emits.
    const unlisten = await listen<FileChangeEvent>(
      "file-changed",
      (event: TauriEvent<FileChangeEvent>) => {
        // Only forward events that are inside the watched directory.
        if (event.payload.path.startsWith(path)) {
          callback(event.payload);
        }
      }
    );

    // Return a cleanup function that both unwatches and removes the listener.
    return async () => {
      unlisten();
      try {
        await invoke("unwatch_directory", { path });
      } catch {
        // Ignore errors on unwatch (e.g. path no longer exists).
      }
    };
  }

  // ── Utility helpers ────────────────────────────────────────────────────────

  /**
   * Derive a language ID string from a file extension for Monaco Editor.
   */
  getLanguageFromExtension(ext: string): string {
    const map: Record<string, string> = {
      ts: "typescript",
      tsx: "typescript",
      js: "javascript",
      jsx: "javascript",
      html: "html",
      htm: "html",
      css: "css",
      scss: "scss",
      less: "less",
      json: "json",
      md: "markdown",
      rs: "rust",
      toml: "toml",
      yaml: "yaml",
      yml: "yaml",
      sh: "shell",
      bash: "shell",
      sql: "sql",
      php: "php",
      py: "python",
      rb: "ruby",
      go: "go",
      c: "c",
      cpp: "cpp",
      h: "cpp",
      java: "java",
      xml: "xml",
      svg: "xml",
      txt: "plaintext",
    };
    return map[ext.toLowerCase()] ?? "plaintext";
  }

  /**
   * Return a human-readable file size string (e.g. "12.3 KB").
   */
  formatFileSize(bytes: number): string {
    if (bytes === 0) return "0 B";
    const units = ["B", "KB", "MB", "GB"];
    const i = Math.floor(Math.log(bytes) / Math.log(1024));
    return `${(bytes / Math.pow(1024, i)).toFixed(1)} ${units[i]}`;
  }

  /**
   * Join path segments with the OS separator.
   * We use "/" universally — Tauri / Rust handles both separators.
   */
  joinPath(...parts: string[]): string {
    return parts
      .filter(Boolean)
      .join("/")
      .replace(/\/+/g, "/");
  }

  /**
   * Extract the parent directory from an absolute path.
   */
  dirname(path: string): string {
    const parts = path.replace(/\\/g, "/").split("/");
    parts.pop();
    return parts.join("/") || "/";
  }

  /**
   * Extract the filename (with extension) from an absolute path.
   */
  basename(path: string): string {
    return path.replace(/\\/g, "/").split("/").pop() ?? "";
  }
}

/** Singleton instance shared throughout the application. */
export const fileService = new FileService();
