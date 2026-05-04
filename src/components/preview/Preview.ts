/**
 * Preview.ts
 * Live-preview panel that renders the current HTML file (or project root)
 * inside an iframe.  Reloads automatically when the file watcher fires.
 */

import { fileService } from "../../services/fileService";

export class PreviewPanel {
  private container: HTMLElement;
  private iframe!: HTMLIFrameElement;
  private urlInput!: HTMLInputElement;
  /** The currently previewed file path (if using blob preview). */
  private currentPath: string | null = null;
  /** Blob URL of the last preview to clean up. */
  private blobUrl: string | null = null;

  constructor(container: HTMLElement) {
    this.container = container;
    this._buildDOM();
  }

  // ── DOM ────────────────────────────────────────────────────────────────────

  private _buildDOM(): void {
    this.container.classList.add("preview-panel");
    this.container.innerHTML = `
      <div class="preview-toolbar">
        <button class="btn" id="btn-reload" title="Reload preview">↺ Reload</button>
        <input class="url-bar" id="preview-url" type="text" placeholder="Preview URL or file…" readonly />
        <button class="btn" id="btn-clear" title="Clear preview">✕</button>
      </div>
      <iframe
        class="preview-iframe"
        sandbox="allow-scripts allow-same-origin allow-forms allow-modals"
        title="Live Preview"
      ></iframe>
    `;

    this.iframe = this.container.querySelector(".preview-iframe")!;
    this.urlInput = this.container.querySelector("#preview-url")!;

    this.container
      .querySelector("#btn-reload")!
      .addEventListener("click", () => this._reload());

    this.container
      .querySelector("#btn-clear")!
      .addEventListener("click", () => this.clear());
  }

  // ── Public API ─────────────────────────────────────────────────────────────

  /**
   * Preview an HTML file by reading it from disk and loading it as a blob URL.
   * Non-HTML files are displayed as plain text.
   */
  async previewFile(path: string): Promise<void> {
    this.currentPath = path;
    await this._loadFromDisk(path);
  }

  /**
   * Called by the file watcher when a file changes.
   * Reloads the preview if the changed file matches the current preview.
   */
  onFileChanged(changedPath: string): void {
    if (this.currentPath && changedPath === this.currentPath) {
      this._reload();
    }
  }

  /** Force-reload the current preview. */
  private async _reload(): Promise<void> {
    if (this.currentPath) {
      await this._loadFromDisk(this.currentPath);
    }
  }

  /** Clear the preview iframe. */
  clear(): void {
    this._revokeBlobUrl();
    this.iframe.src = "about:blank";
    this.urlInput.value = "";
    this.currentPath = null;
  }

  // ── Internals ──────────────────────────────────────────────────────────────

  private async _loadFromDisk(path: string): Promise<void> {
    try {
      const content = await fileService.readFile(path);
      const ext = path.split(".").pop()?.toLowerCase() ?? "";
      const isHtml = ext === "html" || ext === "htm";

      // Revoke any previous blob URL.
      this._revokeBlobUrl();

      const mime = isHtml ? "text/html" : "text/plain";
      const blob = new Blob([content], { type: `${mime};charset=utf-8` });
      this.blobUrl = URL.createObjectURL(blob);

      this.iframe.src = this.blobUrl;
      this.urlInput.value = fileService.basename(path);
    } catch (err) {
      this.iframe.srcdoc = `
        <html><body style="font-family:monospace;color:#f38ba8;padding:1em;">
          <h3>Preview Error</h3><pre>${String(err)}</pre>
        </body></html>
      `;
    }
  }

  private _revokeBlobUrl(): void {
    if (this.blobUrl) {
      URL.revokeObjectURL(this.blobUrl);
      this.blobUrl = null;
    }
  }
}
