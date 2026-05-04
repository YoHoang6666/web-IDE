/**
 * Editor.ts
 * Wraps Monaco Editor with a tab bar, keyboard shortcuts, and auto-save.
 * The editor is a full-featured code editing panel for the IDE.
 */

import * as monaco from "monaco-editor";
import { TabManager, Tab } from "./TabManager";
import { fileService } from "../../services/fileService";
import { showToast } from "../../layout/SplitView";

// ─────────────────────────────────────────────────────────────────────────────
// Monaco global configuration (run once at module load)
// ─────────────────────────────────────────────────────────────────────────────

monaco.editor.defineTheme("ide-dark", {
  base: "vs-dark",
  inherit: true,
  rules: [
    { token: "comment", foreground: "6c7086", fontStyle: "italic" },
    { token: "keyword", foreground: "cba6f7" },
    { token: "string", foreground: "a6e3a1" },
    { token: "number", foreground: "fab387" },
    { token: "type", foreground: "89dceb" },
    { token: "function", foreground: "89b4fa" },
    { token: "variable", foreground: "cdd6f4" },
  ],
  colors: {
    "editor.background": "#1e1e2e",
    "editor.foreground": "#cdd6f4",
    "editorLineNumber.foreground": "#45475a",
    "editorLineNumber.activeForeground": "#6c7086",
    "editor.selectionBackground": "#313244",
    "editor.lineHighlightBackground": "#181825",
    "editorCursor.foreground": "#f5c2e7",
    "editor.wordHighlightBackground": "#313244",
    "editorWidget.background": "#181825",
    "editorSuggestWidget.background": "#181825",
    "editorSuggestWidget.border": "#45475a",
    "editorSuggestWidget.selectedBackground": "#313244",
    "inputValidation.errorBackground": "#f38ba8",
    "tab.activeBackground": "#1e1e2e",
    "tab.inactiveBackground": "#181825",
    "scrollbarSlider.background": "#45475a66",
    "scrollbarSlider.hoverBackground": "#45475a99",
    "minimap.background": "#181825",
  },
});

// ─────────────────────────────────────────────────────────────────────────────
// EditorPanel
// ─────────────────────────────────────────────────────────────────────────────

export class EditorPanel {
  private container: HTMLElement;
  private tabBar!: HTMLElement;
  private monacoHost!: HTMLElement;
  private editor: monaco.editor.IStandaloneCodeEditor | null = null;
  private tabManager: TabManager;
  private onFileSaved?: (path: string, content: string) => void;

  constructor(
    container: HTMLElement,
    options: {
      onFileSaved?: (path: string, content: string) => void;
    } = {}
  ) {
    this.container = container;
    this.tabManager = new TabManager();
    this.onFileSaved = options.onFileSaved;
    this._buildDOM();
    this._initMonaco();
    this._attachTabSubscription();
    this._attachKeyboardShortcuts();
  }

  // ── DOM construction ───────────────────────────────────────────────────────

  private _buildDOM(): void {
    this.container.classList.add("editor-panel");
    this.container.innerHTML = `
      <div class="tab-bar" role="tablist" aria-label="Open files">
        <button class="new-tab-btn" title="New file" aria-label="New file">＋</button>
      </div>
      <div class="monaco-host" style="flex:1;"></div>
    `;
    this.tabBar = this.container.querySelector(".tab-bar")!;
    this.monacoHost = this.container.querySelector(".monaco-host")!;

    // New file button.
    this.tabBar.querySelector(".new-tab-btn")!.addEventListener("click", () => {
      this._createNewFile();
    });
  }

  // ── Monaco initialisation ──────────────────────────────────────────────────

  private _initMonaco(): void {
    this.editor = monaco.editor.create(this.monacoHost, {
      theme: "ide-dark",
      automaticLayout: true,
      fontSize: 14,
      fontFamily: "'JetBrains Mono', 'Fira Code', 'Cascadia Code', monospace",
      fontLigatures: true,
      lineHeight: 22,
      minimap: { enabled: true },
      scrollBeyondLastLine: false,
      wordWrap: "off",
      tabSize: 2,
      insertSpaces: true,
      renderLineHighlight: "all",
      smoothScrolling: true,
      cursorBlinking: "expand",
      cursorSmoothCaretAnimation: "on",
      bracketPairColorization: { enabled: true },
      guides: { bracketPairs: true, indentation: true },
      formatOnPaste: true,
      renderWhitespace: "boundary",
      occurrencesHighlight: "singleFile",
      suggest: { showKeywords: true },
      quickSuggestions: { other: true, comments: false, strings: false },
    });

    // Track content changes to mark the tab as modified.
    this.editor.onDidChangeModelContent(() => {
      const activeTab = this.tabManager.getActiveTab();
      if (activeTab) {
        this.tabManager.markModified(activeTab.id, true);
      }
    });
  }

  // ── Tab bar rendering ──────────────────────────────────────────────────────

  private _attachTabSubscription(): void {
    this.tabManager.subscribe((tabs, activeId) => {
      this._renderTabBar(tabs, activeId);
      // Swap the Monaco model for the active tab.
      if (activeId) {
        const active = this.tabManager.getActiveTab();
        if (active && this.editor) {
          this.editor.setModel(active.model);
          this.editor.focus();
        }
      } else if (this.editor) {
        this.editor.setModel(null);
      }
    });
  }

  private _renderTabBar(tabs: Tab[], activeId: string | null): void {
    // Preserve the new-tab button.
    const newBtn = this.tabBar.querySelector(".new-tab-btn")!;
    this.tabBar.innerHTML = "";
    this.tabBar.appendChild(newBtn);

    for (const tab of tabs) {
      const el = document.createElement("div");
      el.className = `tab-item${tab.id === activeId ? " active" : ""}${tab.modified ? " modified" : ""}`;
      el.setAttribute("role", "tab");
      el.setAttribute("aria-selected", tab.id === activeId ? "true" : "false");
      el.dataset.tabId = tab.id;
      el.innerHTML = `
        <span class="tab-name" title="${tab.path}">${tab.name}</span>
        <button class="tab-close" title="Close tab" aria-label="Close ${tab.name}">×</button>
      `;

      // Activate on click.
      el.addEventListener("click", (e) => {
        if ((e.target as HTMLElement).classList.contains("tab-close")) return;
        this.tabManager.activateTab(tab.id);
      });

      // Close on × button.
      el.querySelector(".tab-close")!.addEventListener("click", (e) => {
        e.stopPropagation();
        this._handleCloseTab(tab.id);
      });

      // Middle-click to close.
      el.addEventListener("auxclick", (e) => {
        if ((e as MouseEvent).button === 1) this._handleCloseTab(tab.id);
      });

      this.tabBar.insertBefore(el, newBtn);
    }
  }

  private async _handleCloseTab(id: string): Promise<void> {
    const tab = this.tabManager.getAllTabs().find((t) => t.id === id);
    if (tab?.modified) {
      const confirmed = window.confirm(
        `"${tab.name}" has unsaved changes. Close anyway?`
      );
      if (!confirmed) return;
    }
    this.tabManager.closeTab(id);
  }

  // ── Keyboard shortcuts ─────────────────────────────────────────────────────

  private _attachKeyboardShortcuts(): void {
    if (!this.editor) return;

    // Ctrl/Cmd + S → save
    this.editor.addCommand(
      monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyS,
      () => this._saveActiveTab()
    );

    // Ctrl/Cmd + W → close active tab
    this.editor.addCommand(
      monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyW,
      () => {
        const tab = this.tabManager.getActiveTab();
        if (tab) this._handleCloseTab(tab.id);
      }
    );
  }

  // ── Save ───────────────────────────────────────────────────────────────────

  private async _saveActiveTab(): Promise<void> {
    const tab = this.tabManager.getActiveTab();
    if (!tab) return;

    const content = tab.model.getValue();
    try {
      await fileService.writeFile(tab.path, content);
      this.tabManager.markSaved(tab.path);
      if (this.onFileSaved) this.onFileSaved(tab.path, content);
      showToast(`Saved: ${tab.name}`, "success");
    } catch (err) {
      showToast(`Failed to save: ${String(err)}`, "error");
    }
  }

  // ── New untitled file ──────────────────────────────────────────────────────

  private _untitledCounter = 0;

  private _createNewFile(): void {
    const name = `untitled-${++this._untitledCounter}.txt`;
    const pseudoPath = `/untitled/${name}`;
    this.openFile(pseudoPath, "", "plaintext");
  }

  // ── Public API ─────────────────────────────────────────────────────────────

  /** Open a file in a new (or existing) tab. */
  openFile(path: string, content: string, language?: string): void {
    const ext = path.split(".").pop() ?? "";
    const lang = language ?? fileService.getLanguageFromExtension(ext);
    this.tabManager.openTab(path, content, lang);
    this.editor?.focus();
  }

  /** Update the content of an already-open tab (e.g. after file change on disk). */
  reloadFile(path: string, content: string): void {
    const tab = this.tabManager.getTabByPath(path);
    if (tab) {
      tab.model.setValue(content);
      this.tabManager.markModified(tab.id, false);
    }
  }

  /** Force layout recalculation (call after the container is resized). */
  layout(): void {
    this.editor?.layout();
  }

  /** Get the Monaco editor instance for advanced usage. */
  getMonacoEditor(): monaco.editor.IStandaloneCodeEditor | null {
    return this.editor;
  }

  /** Get the tab manager for advanced usage. */
  getTabManager(): TabManager {
    return this.tabManager;
  }
}
