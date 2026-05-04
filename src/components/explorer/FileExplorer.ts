/**
 * FileExplorer.ts
 * Renders the file-system tree in the sidebar.  Supports:
 *  - Expanding / collapsing directories
 *  - Opening files in the editor
 *  - Creating new files and folders (inline modal)
 *  - Renaming entries (inline)
 *  - Deleting entries
 *  - Real-time updates via the file watcher
 */

import { fileService, FileEntry } from "../../services/fileService";
import { showToast } from "../../layout/SplitView";

export type OpenFileCallback = (path: string, content: string) => void;

// ─────────────────────────────────────────────────────────────────────────────
// FileExplorer
// ─────────────────────────────────────────────────────────────────────────────

export class FileExplorer {
  private container: HTMLElement;
  private rootPath: string | null = null;
  private onOpenFile: OpenFileCallback;
  private expandedPaths: Set<string> = new Set();
  private unwatch: (() => void) | null = null;
  private pathLabel!: HTMLElement;
  private treeContainer!: HTMLElement;

  constructor(container: HTMLElement, onOpenFile: OpenFileCallback) {
    this.container = container;
    this.onOpenFile = onOpenFile;
    this._buildDOM();
  }

  // ── DOM ────────────────────────────────────────────────────────────────────

  private _buildDOM(): void {
    this.container.classList.add("file-explorer");
    this.container.innerHTML = `
      <div class="explorer-header">
        <span class="path-label" title="">No folder open</span>
        <button class="icon-btn" id="btn-open-folder" title="Open Folder">📂</button>
        <button class="icon-btn" id="btn-new-file"   title="New File">📄</button>
        <button class="icon-btn" id="btn-new-folder" title="New Folder">📁</button>
        <button class="icon-btn" id="btn-refresh"    title="Refresh">🔄</button>
      </div>
      <div class="file-tree" role="tree" aria-label="File explorer"></div>
    `;

    this.pathLabel = this.container.querySelector(".path-label")!;
    this.treeContainer = this.container.querySelector(".file-tree")!;

    this.container
      .querySelector("#btn-open-folder")!
      .addEventListener("click", () => this._openFolder());

    this.container
      .querySelector("#btn-new-file")!
      .addEventListener("click", () =>
        this.rootPath && this._promptNewEntry(this.rootPath, false)
      );

    this.container
      .querySelector("#btn-new-folder")!
      .addEventListener("click", () =>
        this.rootPath && this._promptNewEntry(this.rootPath, true)
      );

    this.container
      .querySelector("#btn-refresh")!
      .addEventListener("click", () => this.refresh());
  }

  // ── Public API ─────────────────────────────────────────────────────────────

  /** Open a specific folder (bypasses the native dialog). */
  async openPath(path: string): Promise<void> {
    // Stop watching the previous directory.
    if (this.unwatch) {
      await this.unwatch();
      this.unwatch = null;
    }

    this.rootPath = path;
    this.pathLabel.textContent = fileService.basename(path);
    this.pathLabel.title = path;
    this.expandedPaths.add(path);

    await this._renderTree(this.rootPath, this.treeContainer, 0);

    // Start watching for real-time updates.
    try {
      this.unwatch = await fileService.watchDirectory(path, () => {
        this.refresh();
      });
    } catch {
      // Watcher is best-effort; tree still works without it.
    }
  }

  /** Refresh the tree (re-read from disk). */
  async refresh(): Promise<void> {
    if (!this.rootPath) return;
    await this._renderTree(this.rootPath, this.treeContainer, 0);
  }

  // ── Native folder dialog ───────────────────────────────────────────────────

  private async _openFolder(): Promise<void> {
    try {
      const path = await fileService.openFolderDialog();
      if (path) await this.openPath(path);
    } catch (err) {
      showToast(`Failed to open folder: ${String(err)}`, "error");
    }
  }

  // ── Tree rendering ─────────────────────────────────────────────────────────

  private async _renderTree(
    dirPath: string,
    container: HTMLElement,
    depth: number
  ): Promise<void> {
    let entries: FileEntry[];
    try {
      entries = await fileService.listDirectory(dirPath);
    } catch {
      container.innerHTML = `<div class="tree-empty">Cannot read directory</div>`;
      return;
    }

    container.innerHTML = "";

    if (entries.length === 0) {
      container.innerHTML = `<div class="tree-empty">Empty folder</div>`;
      return;
    }

    for (const entry of entries) {
      const row = this._createTreeItem(entry, depth);
      container.appendChild(row);

      // If this directory was previously expanded, render its children inline.
      if (entry.is_dir && this.expandedPaths.has(entry.path)) {
        const children = document.createElement("div");
        children.className = "tree-children";
        container.appendChild(children);
        await this._renderTree(entry.path, children, depth + 1);
      }
    }
  }

  private _createTreeItem(entry: FileEntry, depth: number): HTMLElement {
    const row = document.createElement("div");
    row.className = "tree-item";
    row.setAttribute("role", entry.is_dir ? "treeitem" : "treeitem");
    row.setAttribute("aria-expanded", this.expandedPaths.has(entry.path) ? "true" : "false");
    row.style.paddingLeft = `${8 + depth * 16}px`;

    const icon = entry.is_dir
      ? this.expandedPaths.has(entry.path)
        ? "📂"
        : "📁"
      : this._fileIcon(entry.extension);

    row.innerHTML = `
      <span class="tree-icon">${icon}</span>
      <span class="tree-label" title="${entry.path}">${entry.name}</span>
      <span class="tree-actions">
        ${!entry.is_dir ? "" : `<button title="New file here" data-action="new-file">+F</button>
                                 <button title="New folder here" data-action="new-folder">+D</button>`}
        <button title="Rename" data-action="rename">✏️</button>
        <button title="Delete" data-action="delete" style="color:var(--text-error)">🗑️</button>
      </span>
    `;

    // ── Click handlers ────────────────────────────────────────────────────────
    row.addEventListener("click", async (e) => {
      const btn = (e.target as HTMLElement).closest("[data-action]") as HTMLElement | null;
      if (btn) {
        e.stopPropagation();
        const action = btn.dataset.action!;
        await this._handleAction(action, entry, row);
        return;
      }

      if (entry.is_dir) {
        await this._toggleDir(entry, row);
      } else {
        await this._openFile(entry);
        // Highlight selected file.
        document.querySelectorAll(".tree-item.selected").forEach((el) =>
          el.classList.remove("selected")
        );
        row.classList.add("selected");
      }
    });

    return row;
  }

  // ── Directory toggle ───────────────────────────────────────────────────────

  private async _toggleDir(entry: FileEntry, row: HTMLElement): Promise<void> {
    const isExpanded = this.expandedPaths.has(entry.path);

    if (isExpanded) {
      // Collapse: remove children container.
      this.expandedPaths.delete(entry.path);
      const next = row.nextElementSibling;
      if (next?.classList.contains("tree-children")) next.remove();
      row.querySelector(".tree-icon")!.textContent = "📁";
      row.setAttribute("aria-expanded", "false");
    } else {
      // Expand: add children container.
      this.expandedPaths.add(entry.path);
      row.querySelector(".tree-icon")!.textContent = "📂";
      row.setAttribute("aria-expanded", "true");

      const children = document.createElement("div");
      children.className = "tree-children";
      row.insertAdjacentElement("afterend", children);

      const depth = parseInt(row.style.paddingLeft) / 16 - 0; // rough depth calc
      await this._renderTree(entry.path, children, depth + 1);
    }
  }

  // ── Open file ──────────────────────────────────────────────────────────────

  private async _openFile(entry: FileEntry): Promise<void> {
    try {
      const content = await fileService.readFile(entry.path);
      this.onOpenFile(entry.path, content);
    } catch (err) {
      showToast(`Cannot open file: ${String(err)}`, "error");
    }
  }

  // ── Context actions ────────────────────────────────────────────────────────

  private async _handleAction(
    action: string,
    entry: FileEntry,
    _row: HTMLElement
  ): Promise<void> {
    switch (action) {
      case "new-file":
        await this._promptNewEntry(entry.path, false);
        break;
      case "new-folder":
        await this._promptNewEntry(entry.path, true);
        break;
      case "rename":
        await this._promptRename(entry);
        break;
      case "delete":
        await this._confirmDelete(entry);
        break;
    }
  }

  private async _promptNewEntry(dirPath: string, isDir: boolean): Promise<void> {
    const kind = isDir ? "folder" : "file";
    const name = window.prompt(`New ${kind} name:`);
    if (!name?.trim()) return;

    const newPath = fileService.joinPath(dirPath, name.trim());
    try {
      if (isDir) {
        await fileService.createDirectory(newPath);
      } else {
        await fileService.writeFile(newPath, "");
      }
      await this.refresh();
      showToast(`Created: ${name}`, "success");
    } catch (err) {
      showToast(`Failed to create: ${String(err)}`, "error");
    }
  }

  private async _promptRename(entry: FileEntry): Promise<void> {
    const newName = window.prompt("New name:", entry.name);
    if (!newName?.trim() || newName.trim() === entry.name) return;

    const newPath = fileService.joinPath(
      fileService.dirname(entry.path),
      newName.trim()
    );
    try {
      await fileService.renameEntry(entry.path, newPath);
      await this.refresh();
      showToast(`Renamed to: ${newName}`, "success");
    } catch (err) {
      showToast(`Failed to rename: ${String(err)}`, "error");
    }
  }

  private async _confirmDelete(entry: FileEntry): Promise<void> {
    const confirmed = window.confirm(
      `Delete "${entry.name}"?${entry.is_dir ? "\n(This will delete all contents.)" : ""}`
    );
    if (!confirmed) return;

    try {
      await fileService.deleteFile(entry.path);
      await this.refresh();
      showToast(`Deleted: ${entry.name}`, "info");
    } catch (err) {
      showToast(`Failed to delete: ${String(err)}`, "error");
    }
  }

  // ── Helpers ────────────────────────────────────────────────────────────────

  private _fileIcon(ext: string): string {
    const icons: Record<string, string> = {
      ts: "🟦", tsx: "🟦", js: "🟨", jsx: "🟨",
      html: "🌐", htm: "🌐", css: "🎨", scss: "🎨", less: "🎨",
      json: "📋", md: "📝", rs: "🦀", toml: "⚙️",
      sql: "🗄️", php: "🐘", py: "🐍", rb: "💎",
      sh: "💻", bash: "💻", yaml: "⚙️", yml: "⚙️",
      png: "🖼️", jpg: "🖼️", jpeg: "🖼️", gif: "🖼️", svg: "🖼️",
      pdf: "📕", zip: "📦", gz: "📦",
    };
    return icons[ext.toLowerCase()] ?? "📄";
  }
}
