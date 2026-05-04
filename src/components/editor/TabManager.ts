/**
 * TabManager.ts
 * Manages open editor tabs: creation, activation, modification state, and closure.
 * Each tab holds a reference to its Monaco ITextModel.
 */

import * as monaco from "monaco-editor";

// ─────────────────────────────────────────────────────────────────────────────
// Types
// ─────────────────────────────────────────────────────────────────────────────

export interface Tab {
  /** Unique id for the tab (randomly generated). */
  id: string;
  /** Full file path (used as the Monaco model URI). */
  path: string;
  /** Display name shown on the tab. */
  name: string;
  /** File language id. */
  language: string;
  /** Whether the file has unsaved changes. */
  modified: boolean;
  /** The Monaco text model associated with this tab. */
  model: monaco.editor.ITextModel;
}

export type TabChangeCallback = (tabs: Tab[], activeId: string | null) => void;

// ─────────────────────────────────────────────────────────────────────────────
// TabManager
// ─────────────────────────────────────────────────────────────────────────────

export class TabManager {
  private tabs: Map<string, Tab> = new Map();
  private activeTabId: string | null = null;
  private listeners: Set<TabChangeCallback> = new Set();
  private idCounter = 0;

  // ── Subscriptions ──────────────────────────────────────────────────────────

  /** Subscribe to any tab state change.  Returns an unsubscribe function. */
  subscribe(cb: TabChangeCallback): () => void {
    this.listeners.add(cb);
    return () => this.listeners.delete(cb);
  }

  private emit(): void {
    const list = [...this.tabs.values()];
    this.listeners.forEach((cb) => cb(list, this.activeTabId));
  }

  // ── Tab operations ─────────────────────────────────────────────────────────

  /**
   * Open a new tab for `path`, or activate an existing one.
   * Returns the tab id.
   */
  openTab(path: string, content: string, language: string): string {
    // Reuse an existing tab for the same file.
    for (const tab of this.tabs.values()) {
      if (tab.path === path) {
        this.activateTab(tab.id);
        return tab.id;
      }
    }

    const id = `tab-${++this.idCounter}`;
    const uri = monaco.Uri.file(path);

    // Reuse an existing model (Monaco may already have one for this URI).
    let model = monaco.editor.getModel(uri);
    if (!model) {
      model = monaco.editor.createModel(content, language, uri);
    } else {
      // Update the model content if the file was changed outside of Monaco.
      if (model.getValue() !== content) {
        model.setValue(content);
      }
    }

    const name = path.split(/[\\/]/).pop() ?? path;

    const tab: Tab = { id, path, name, language, modified: false, model };
    this.tabs.set(id, tab);
    this.activeTabId = id;
    this.emit();
    return id;
  }

  /** Close a tab by id.  Activates the nearest remaining tab if needed. */
  closeTab(id: string): void {
    const tab = this.tabs.get(id);
    if (!tab) return;

    // Dispose Monaco model to free memory.
    tab.model.dispose();
    this.tabs.delete(id);

    if (this.activeTabId === id) {
      // Try to activate the previous / next tab.
      const remaining = [...this.tabs.keys()];
      this.activeTabId = remaining.length > 0 ? remaining[remaining.length - 1] : null;
    }

    this.emit();
  }

  /** Make `id` the active tab. */
  activateTab(id: string): void {
    if (this.tabs.has(id)) {
      this.activeTabId = id;
      this.emit();
    }
  }

  /** Mark a tab as modified (unsaved changes). */
  markModified(id: string, modified: boolean): void {
    const tab = this.tabs.get(id);
    if (tab) {
      tab.modified = modified;
      this.emit();
    }
  }

  /** Mark the tab that owns `path` as unmodified (after a save). */
  markSaved(path: string): void {
    for (const tab of this.tabs.values()) {
      if (tab.path === path) {
        tab.modified = false;
      }
    }
    this.emit();
  }

  // ── Getters ────────────────────────────────────────────────────────────────

  getActiveTab(): Tab | null {
    return this.activeTabId ? (this.tabs.get(this.activeTabId) ?? null) : null;
  }

  getTabByPath(path: string): Tab | undefined {
    for (const tab of this.tabs.values()) {
      if (tab.path === path) return tab;
    }
    return undefined;
  }

  getAllTabs(): Tab[] {
    return [...this.tabs.values()];
  }

  get activeId(): string | null {
    return this.activeTabId;
  }
}
