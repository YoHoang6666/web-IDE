/**
 * SplitView.ts
 * Resizable split-view container and global toast notification utility.
 *
 * Provides:
 *  - SplitView class: two panes separated by a draggable divider
 *  - showToast(): global toast notification helper
 */

// ─────────────────────────────────────────────────────────────────────────────
// Toast notification system
// ─────────────────────────────────────────────────────────────────────────────

let _toastContainer: HTMLElement | null = null;

function _getToastContainer(): HTMLElement {
  if (!_toastContainer) {
    _toastContainer = document.createElement("div");
    _toastContainer.className = "toast-container";
    document.body.appendChild(_toastContainer);
  }
  return _toastContainer;
}

/**
 * Show a temporary toast notification.
 * @param message  The message to display.
 * @param type     "success" | "error" | "info" (controls the accent colour).
 * @param duration Duration in milliseconds before auto-dismissal (default 3000).
 */
export function showToast(
  message: string,
  type: "success" | "error" | "info" = "info",
  duration = 3000
): void {
  const container = _getToastContainer();

  const toast = document.createElement("div");
  toast.className = `toast ${type}`;
  toast.textContent = message;

  container.appendChild(toast);

  setTimeout(() => {
    toast.style.opacity = "0";
    toast.style.transition = "opacity 0.3s";
    setTimeout(() => toast.remove(), 300);
  }, duration);
}

// ─────────────────────────────────────────────────────────────────────────────
// SplitView
// ─────────────────────────────────────────────────────────────────────────────

export type SplitDirection = "vertical" | "horizontal";

export interface SplitViewOptions {
  /** "vertical" = left | right, "horizontal" = top | bottom */
  direction?: SplitDirection;
  /** Initial size of the first pane as a fraction [0, 1] (default 0.5). */
  initialRatio?: number;
  /** Minimum fraction for the first pane (default 0.1). */
  minRatio?: number;
  /** Maximum fraction for the first pane (default 0.9). */
  maxRatio?: number;
  /** Called each time the divider is dragged. */
  onResize?: (ratio: number) => void;
}

export class SplitView {
  readonly container: HTMLElement;
  readonly paneA: HTMLElement;
  readonly paneB: HTMLElement;
  private resizer: HTMLElement;
  private direction: SplitDirection;
  private ratio: number;
  private minRatio: number;
  private maxRatio: number;
  private onResize?: (ratio: number) => void;

  constructor(container: HTMLElement, options: SplitViewOptions = {}) {
    this.container = container;
    this.direction = options.direction ?? "vertical";
    this.ratio = options.initialRatio ?? 0.5;
    this.minRatio = options.minRatio ?? 0.1;
    this.maxRatio = options.maxRatio ?? 0.9;
    this.onResize = options.onResize;

    // Build the DOM structure.
    this.container.classList.add("split-container", this.direction);

    this.paneA = document.createElement("div");
    this.paneA.className = "split-pane";

    this.resizer = document.createElement("div");
    this.resizer.className = "split-resizer";

    this.paneB = document.createElement("div");
    this.paneB.className = "split-pane";

    this.container.appendChild(this.paneA);
    this.container.appendChild(this.resizer);
    this.container.appendChild(this.paneB);

    this._applyRatio();
    this._attachDragListeners();
  }

  // ── Layout ─────────────────────────────────────────────────────────────────

  private _applyRatio(): void {
    const pct = (this.ratio * 100).toFixed(2) + "%";
    const rem = ((1 - this.ratio) * 100).toFixed(2) + "%";

    if (this.direction === "vertical") {
      this.paneA.style.width = pct;
      this.paneA.style.height = "100%";
      this.paneB.style.width = rem;
      this.paneB.style.height = "100%";
      this.paneA.style.flex = "none";
      this.paneB.style.flex = "none";
    } else {
      this.paneA.style.height = pct;
      this.paneA.style.width = "100%";
      this.paneB.style.height = rem;
      this.paneB.style.width = "100%";
      this.paneA.style.flex = "none";
      this.paneB.style.flex = "none";
    }
  }

  // ── Drag-to-resize ─────────────────────────────────────────────────────────

  private _attachDragListeners(): void {
    let dragging = false;
    let startPos = 0;
    let startRatio = 0;

    const onMouseMove = (e: MouseEvent): void => {
      if (!dragging) return;

      const rect = this.container.getBoundingClientRect();
      let delta: number;
      let total: number;

      if (this.direction === "vertical") {
        delta = e.clientX - startPos;
        total = rect.width;
      } else {
        delta = e.clientY - startPos;
        total = rect.height;
      }

      const newRatio = Math.min(
        this.maxRatio,
        Math.max(this.minRatio, startRatio + delta / total)
      );

      if (newRatio !== this.ratio) {
        this.ratio = newRatio;
        this._applyRatio();
        this.onResize?.(newRatio);
      }
    };

    const onMouseUp = (): void => {
      if (!dragging) return;
      dragging = false;
      this.resizer.classList.remove("dragging");
      document.body.style.cursor = "";
      document.body.style.userSelect = "";
    };

    this.resizer.addEventListener("mousedown", (e) => {
      dragging = true;
      startPos = this.direction === "vertical" ? e.clientX : e.clientY;
      startRatio = this.ratio;
      this.resizer.classList.add("dragging");
      document.body.style.cursor =
        this.direction === "vertical" ? "col-resize" : "row-resize";
      document.body.style.userSelect = "none";
      e.preventDefault();
    });

    document.addEventListener("mousemove", onMouseMove);
    document.addEventListener("mouseup", onMouseUp);
  }

  // ── Public API ─────────────────────────────────────────────────────────────

  /** Set the split ratio programmatically. */
  setRatio(ratio: number): void {
    this.ratio = Math.min(this.maxRatio, Math.max(this.minRatio, ratio));
    this._applyRatio();
  }

  /** Show or hide paneB (collapses to/from the right/bottom). */
  setPaneBVisible(visible: boolean): void {
    if (visible) {
      this.paneB.style.display = "";
      this.resizer.style.display = "";
      this._applyRatio();
    } else {
      this.paneB.style.display = "none";
      this.resizer.style.display = "none";
      if (this.direction === "vertical") {
        this.paneA.style.width = "100%";
      } else {
        this.paneA.style.height = "100%";
      }
    }
  }
}
