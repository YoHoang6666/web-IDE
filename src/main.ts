/**
 * main.ts
 * Application entry point.
 * Bootstraps the IDE layout, wires all components together, and registers
 * global keyboard shortcuts.
 *
 * Layout hierarchy:
 *
 *   #app
 *   └── .ide-layout
 *       ├── .title-bar          ← menu / actions / current project name
 *       ├── .main-area
 *       │   ├── .sidebar        ← file explorer + DB list (tabbed)
 *       │   └── .content-area
 *       │       └── SplitView (vertical)
 *       │           ├── paneA  ← EditorPanel
 *       │           └── paneB  ← PreviewPanel | DatabasePanel (switchable)
 *       └── .status-bar         ← status items
 */

import { EditorPanel } from "./components/editor/Editor";
import { FileExplorer } from "./components/explorer/FileExplorer";
import { PreviewPanel } from "./components/preview/Preview";
import { DatabasePanel } from "./components/database/DatabasePanel";
import { SplitView, showToast } from "./layout/SplitView";
import { fileService } from "./services/fileService";

// ─────────────────────────────────────────────────────────────────────────────
// Build the static HTML skeleton
// ─────────────────────────────────────────────────────────────────────────────

function buildSkeleton(root: HTMLElement): void {
  root.innerHTML = `
    <div class="ide-layout">

      <!-- ── Title bar ──────────────────────────────────────────────────── -->
      <header class="title-bar" aria-label="Application menu">
        <span class="logo">⚡ web-IDE</span>

        <nav class="menu-actions" aria-label="View switcher">
          <button class="btn-icon" id="view-editor"   title="Editor only">Editor</button>
          <button class="btn-icon active" id="view-split-preview" title="Editor + Preview">+ Preview</button>
          <button class="btn-icon" id="view-split-db"  title="Editor + Database">+ Database</button>
        </nav>

        <div class="spacer"></div>
        <span class="current-project" id="current-project" title="">No project open</span>
      </header>

      <!-- ── Main area ──────────────────────────────────────────────────── -->
      <div class="main-area">

        <!-- Sidebar -->
        <aside class="sidebar" aria-label="Sidebar">
          <div class="sidebar-tabs" role="tablist">
            <div class="s-tab active" role="tab" aria-selected="true"  data-panel="files"    id="stab-files">Files</div>
            <div class="s-tab"        role="tab" aria-selected="false" data-panel="database" id="stab-database">Database</div>
          </div>
          <div class="sidebar-panel active" id="spanel-files"   role="tabpanel"></div>
          <div class="sidebar-panel"        id="spanel-database" role="tabpanel"></div>
        </aside>

        <!-- Content area -->
        <main class="content-area" id="content-area" aria-label="Editor area"></main>
      </div>

      <!-- ── Status bar ─────────────────────────────────────────────────── -->
      <footer class="status-bar" aria-label="Status bar">
        <span class="status-item" id="status-lang">Plain Text</span>
        <span class="status-sep"></span>
        <span class="status-item" id="status-cursor">Ln 1, Col 1</span>
        <span class="status-sep"></span>
        <span class="status-item" id="status-encoding">UTF-8</span>
        <span class="spacer"></span>
        <span class="status-item" id="status-project">web-IDE v0.1</span>
      </footer>
    </div>
  `;
}

// ─────────────────────────────────────────────────────────────────────────────
// Bootstrap
// ─────────────────────────────────────────────────────────────────────────────

async function init(): Promise<void> {
  const appRoot = document.getElementById("app");
  if (!appRoot) throw new Error("#app element not found");

  // Render static skeleton.
  buildSkeleton(appRoot);

  // ── Grab DOM references ──────────────────────────────────────────────────
  const contentArea      = document.getElementById("content-area")!;
  const sFilesPanel      = document.getElementById("spanel-files")!;
  const sDatabasePanel   = document.getElementById("spanel-database")!;
  const statusLang       = document.getElementById("status-lang")!;
  const statusCursor     = document.getElementById("status-cursor")!;
  const currentProject   = document.getElementById("current-project")!;

  // ── Split view ────────────────────────────────────────────────────────────
  const split = new SplitView(contentArea, {
    direction: "vertical",
    initialRatio: 0.55,
    minRatio: 0.2,
    maxRatio: 0.85,
    onResize: () => {
      editorPanel.layout();
    },
  });

  // ── Editor ────────────────────────────────────────────────────────────────
  const editorPanel = new EditorPanel(split.paneA, {
    onFileSaved: (path, _content) => {
      // After saving an HTML file, update the preview.
      if (/\.(html?|css|js)$/i.test(path)) {
        previewPanel.onFileChanged(path);
      }
    },
  });

  // ── Preview panel ─────────────────────────────────────────────────────────
  const previewPanel = new PreviewPanel(split.paneB);

  // ── Database panel ────────────────────────────────────────────────────────
  // The DB panel is mounted but hidden until the user switches to DB view.
  const dbPaneContainer = document.createElement("div");
  dbPaneContainer.style.cssText = "display:none;flex:1;overflow:hidden;";
  split.paneB.appendChild(dbPaneContainer);
  const databasePanel = new DatabasePanel(dbPaneContainer);

  // ── File Explorer ─────────────────────────────────────────────────────────
  const fileExplorer = new FileExplorer(sFilesPanel, (path, content) => {
    editorPanel.openFile(path, content);
    // Auto-preview HTML files.
    if (/\.(html?|css)$/i.test(path)) {
      previewPanel.previewFile(path);
    }
    // Update status bar language.
    const ext = path.split(".").pop() ?? "";
    statusLang.textContent =
      fileService.getLanguageFromExtension(ext).replace(/^\w/, (c) => c.toUpperCase());
  });

  // DB list panel in sidebar: shows a hint and an "Open DB" shortcut.
  sDatabasePanel.innerHTML = `
    <div class="db-list-panel">
      <div class="db-list-header">
        <span>Databases</span>
        <button class="icon-btn" title="Switch to DB panel" id="sidebar-db-open">🗄️</button>
      </div>
      <div class="db-list-items">
        <div style="padding:12px;font-size:12px;color:var(--text-muted);">
          Open a database from the Database panel (use the "+ Database" view button in the toolbar).
        </div>
      </div>
    </div>
  `;
  document.getElementById("sidebar-db-open")!.addEventListener("click", () => {
    activateView("db");
  });

  // ── Monaco cursor position → status bar ──────────────────────────────────
  const monacoEditor = editorPanel.getMonacoEditor();
  if (monacoEditor) {
    monacoEditor.onDidChangeCursorPosition((e) => {
      statusCursor.textContent = `Ln ${e.position.lineNumber}, Col ${e.position.column}`;
    });
  }

  // ── View switcher ─────────────────────────────────────────────────────────
  type ViewMode = "editor" | "preview" | "db";

  function activateView(mode: ViewMode): void {
    const btnEditor  = document.getElementById("view-editor")!;
    const btnPreview = document.getElementById("view-split-preview")!;
    const btnDb      = document.getElementById("view-split-db")!;

    // Reset all buttons.
    [btnEditor, btnPreview, btnDb].forEach((b) => b.classList.remove("active"));

    if (mode === "editor") {
      btnEditor.classList.add("active");
      split.setPaneBVisible(false);
    } else if (mode === "preview") {
      btnPreview.classList.add("active");
      // Show preview pane, hide DB pane.
      dbPaneContainer.style.display = "none";
      // The iframe is already in paneB — just make it visible.
      split.setPaneBVisible(true);
      split.paneB.querySelector(".preview-panel")
        ? null
        : split.paneB.insertBefore(
            split.paneB.querySelector(".preview-panel") ?? document.createElement("div"),
            dbPaneContainer
          );
      const previewEl = split.paneB.querySelector(".preview-panel") as HTMLElement | null;
      if (previewEl) {
        previewEl.style.display = "flex";
      }
    } else {
      btnDb.classList.add("active");
      // Show DB pane, hide preview pane.
      const previewEl = split.paneB.querySelector(".preview-panel") as HTMLElement | null;
      if (previewEl) {
        previewEl.style.display = "none";
      }
      dbPaneContainer.style.display = "flex";
      split.setPaneBVisible(true);
    }

    editorPanel.layout();
  }

  document.getElementById("view-editor")!.addEventListener("click", () => activateView("editor"));
  document.getElementById("view-split-preview")!.addEventListener("click", () => activateView("preview"));
  document.getElementById("view-split-db")!.addEventListener("click", () => activateView("db"));

  // Start with preview visible.
  activateView("preview");

  // ── Sidebar tab switching ─────────────────────────────────────────────────
  document.querySelectorAll(".sidebar-tabs .s-tab").forEach((tab) => {
    tab.addEventListener("click", () => {
      const panel = (tab as HTMLElement).dataset.panel!;
      document.querySelectorAll(".sidebar-tabs .s-tab").forEach((t) => {
        t.classList.remove("active");
        t.setAttribute("aria-selected", "false");
      });
      tab.classList.add("active");
      tab.setAttribute("aria-selected", "true");

      document.querySelectorAll(".sidebar-panel").forEach((p) => p.classList.remove("active"));
      document.getElementById(`spanel-${panel}`)!.classList.add("active");
    });
  });

  // ── Global keyboard shortcuts ─────────────────────────────────────────────
  document.addEventListener("keydown", async (e) => {
    // Ctrl/Cmd + O → open folder
    if ((e.ctrlKey || e.metaKey) && e.key === "o") {
      e.preventDefault();
      const path = await fileService.openFolderDialog();
      if (path) {
        await fileExplorer.openPath(path);
        currentProject.textContent = fileService.basename(path);
        currentProject.title = path;
        showToast(`Opened project: ${fileService.basename(path)}`, "success");
      }
    }
  });

  // ── Initial welcome message ───────────────────────────────────────────────
  showToast("web-IDE ready  •  Ctrl+O to open a folder", "info", 5000);

  console.log("[web-IDE] Application initialised");
}

// Kick off.
init().catch((err) => {
  console.error("[web-IDE] Fatal initialisation error:", err);
  document.getElementById("app")!.innerHTML = `
    <div style="padding:2em;color:#f38ba8;font-family:monospace;">
      <h2>Initialisation Error</h2>
      <pre>${String(err)}</pre>
    </div>
  `;
});
