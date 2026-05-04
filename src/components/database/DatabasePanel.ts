/**
 * DatabasePanel.ts
 * Full-featured SQLite database panel:
 *  - Open / switch between databases
 *  - Browse tables with scrollable, paginated data grid
 *  - Inline cell editing with UPDATE generation
 *  - Query runner with syntax-aware textarea
 *  - Table schema viewer
 */

import { invoke } from "@tauri-apps/api/core";
import { dbService, QueryResult, TableInfo } from "../../services/dbService";
import { fileService } from "../../services/fileService";
import { showToast } from "../../layout/SplitView";

export class DatabasePanel {
  private container: HTMLElement;
  private tableList!: HTMLElement;
  private queryTextarea!: HTMLTextAreaElement;
  private resultArea!: HTMLElement;
  private currentTable: string | null = null;
  private currentSchema: TableInfo[] = [];
  private currentResult: QueryResult | null = null;

  constructor(container: HTMLElement) {
    this.container = container;
    this._buildDOM();
  }

  // ── DOM construction ───────────────────────────────────────────────────────

  private _buildDOM(): void {
    this.container.classList.add("db-panel");
    this.container.innerHTML = `
      <div class="db-toolbar">
        <button class="btn primary" id="btn-open-db">📂 Open DB</button>
        <button class="btn" id="btn-new-db">✚ New DB</button>
        <span style="flex:1;"></span>
        <span id="db-name-label" style="font-size:12px;color:var(--text-muted);font-family:var(--font-mono);"></span>
      </div>

      <div class="db-content">
        <!-- Left: table list -->
        <div class="db-sidebar">
          <div class="db-section-title">Tables</div>
          <div id="db-table-list"></div>
        </div>

        <!-- Right: query editor + results -->
        <div class="db-main">
          <div class="query-editor-wrap">
            <div class="query-editor-header">
              <span>SQL Query</span>
              <span style="flex:1;"></span>
              <button class="run-btn" id="btn-run-query">▶ Run (Ctrl+Enter)</button>
            </div>
            <textarea
              id="query-textarea"
              class="query-textarea"
              spellcheck="false"
              placeholder="SELECT * FROM my_table LIMIT 100;"
            ></textarea>
          </div>

          <div id="result-area" class="result-area">
            <div class="result-message">Open a database and select a table, or write a query above.</div>
          </div>
        </div>
      </div>
    `;

    this.tableList = this.container.querySelector("#db-table-list")!;
    this.queryTextarea = this.container.querySelector("#query-textarea")!;
    this.resultArea = this.container.querySelector("#result-area")!;

    // Button handlers.
    this.container
      .querySelector("#btn-open-db")!
      .addEventListener("click", () => this._openDatabase());

    this.container
      .querySelector("#btn-new-db")!
      .addEventListener("click", () => this._createDatabase());

    this.container
      .querySelector("#btn-run-query")!
      .addEventListener("click", () => this._runQuery());

    // Ctrl+Enter in textarea also runs the query.
    this.queryTextarea.addEventListener("keydown", (e) => {
      if ((e.ctrlKey || e.metaKey) && e.key === "Enter") {
        e.preventDefault();
        this._runQuery();
      }
    });

    // Tab key inserts spaces instead of switching focus.
    this.queryTextarea.addEventListener("keydown", (e) => {
      if (e.key === "Tab") {
        e.preventDefault();
        const start = this.queryTextarea.selectionStart;
        const end = this.queryTextarea.selectionEnd;
        this.queryTextarea.value =
          this.queryTextarea.value.substring(0, start) +
          "  " +
          this.queryTextarea.value.substring(end);
        this.queryTextarea.selectionStart = this.queryTextarea.selectionEnd = start + 2;
      }
    });
  }

  // ── Open / create database ─────────────────────────────────────────────────

  private async _openDatabase(): Promise<void> {
    // Use native file dialog via Tauri v2 / rfd (via backend command).
    try {
      const selected = await invoke<string | null>("open_file_dialog", {
        filters: [{ name: "SQLite Database", extensions: ["db", "sqlite", "sqlite3"] }],
      });
      if (!selected) return;
      await this._connectToDatabase(selected);
    } catch (err) {
      showToast(`Cannot open DB: ${String(err)}`, "error");
    }
  }

  private async _createDatabase(): Promise<void> {
    try {
      const selected = await invoke<string | null>("save_file_dialog", {
        defaultName: "new-database.db",
        filters: [{ name: "SQLite Database", extensions: ["db"] }],
      });
      if (!selected) return;
      await this._connectToDatabase(selected);
      showToast(`Created: ${fileService.basename(selected)}`, "success");
    } catch (err) {
      showToast(`Cannot create DB: ${String(err)}`, "error");
    }
  }

  private async _connectToDatabase(path: string): Promise<void> {
    try {
      await dbService.openDatabase(path);
      const label = this.container.querySelector("#db-name-label")!;
      label.textContent = `🗄️ ${fileService.basename(path)}`;
      await this._refreshTableList();
      showToast(`Opened: ${fileService.basename(path)}`, "success");
    } catch (err) {
      showToast(`Failed to open DB: ${String(err)}`, "error");
    }
  }

  // ── Table list ─────────────────────────────────────────────────────────────

  private async _refreshTableList(): Promise<void> {
    let tables: string[];
    try {
      tables = await dbService.listTables();
    } catch (err) {
      showToast(`Cannot list tables: ${String(err)}`, "error");
      return;
    }

    this.tableList.innerHTML = "";
    if (tables.length === 0) {
      this.tableList.innerHTML = `<div style="padding:8px 12px;font-size:12px;color:var(--text-muted);">No tables</div>`;
      return;
    }

    for (const name of tables) {
      const item = document.createElement("div");
      item.className = `db-table-item${this.currentTable === name ? " active" : ""}`;
      item.innerHTML = `<span>⊞</span><span>${name}</span>`;
      item.addEventListener("click", () => this._browseTable(name));
      this.tableList.appendChild(item);
    }
  }

  // ── Browse table ───────────────────────────────────────────────────────────

  private async _browseTable(tableName: string): Promise<void> {
    this.currentTable = tableName;

    // Update active state in sidebar.
    this.tableList.querySelectorAll(".db-table-item").forEach((el) => {
      el.classList.toggle("active", el.querySelector("span:last-child")?.textContent === tableName);
    });

    // Pre-fill the query textarea.
    this.queryTextarea.value = `SELECT * FROM "${tableName}" LIMIT 500;`;

    // Load schema for inline editing.
    try {
      this.currentSchema = await dbService.tableSchema(tableName);
    } catch {
      this.currentSchema = [];
    }

    // Fetch and display rows.
    try {
      const result = await dbService.browseTable(tableName);
      this.currentResult = result;
      this._renderTable(result, true);
    } catch (err) {
      this._showMessage(`Error: ${String(err)}`, "error");
    }
  }

  // ── Run query ──────────────────────────────────────────────────────────────

  private async _runQuery(): Promise<void> {
    const sql = this.queryTextarea.value.trim();
    if (!sql) return;

    // Determine if it's a SELECT or a mutating statement.
    const isSelect = /^\s*SELECT\b/i.test(sql);

    try {
      if (isSelect) {
        const result = await dbService.query(sql);
        this.currentResult = result;
        this._renderTable(result, false);
      } else {
        const affected = await dbService.execute(sql);
        this._showMessage(
          `✓ Query executed successfully. Rows affected: ${affected}`,
          "success"
        );
        // Refresh the table list in case tables were created / dropped.
        await this._refreshTableList();
      }
    } catch (err) {
      this._showMessage(`✗ ${String(err)}`, "error");
    }
  }

  // ── Render result table ────────────────────────────────────────────────────

  private _renderTable(result: QueryResult, editable: boolean): void {
    if (result.rows.length === 0) {
      this._showMessage(
        `No rows returned. (${result.columns.length} columns)`,
        "info" as any
      );
      return;
    }

    // Find primary-key column for editable rows.
    const pkCol = this.currentSchema.find((c) => c.primary_key)?.name ?? null;

    const table = document.createElement("table");
    table.className = "data-table";

    // Header.
    const thead = document.createElement("thead");
    thead.innerHTML = `<tr>${result.columns
      .map((c) => `<th>${this._esc(c)}</th>`)
      .join("")}</tr>`;
    table.appendChild(thead);

    // Body.
    const tbody = document.createElement("tbody");
    for (const row of result.rows) {
      const tr = document.createElement("tr");
      for (const col of result.columns) {
        const td = document.createElement("td");
        const rawValue = row[col];
        const isNull = rawValue === null || rawValue === undefined;

        if (isNull) {
          td.className = "null-val";
          td.textContent = "NULL";
        } else {
          td.textContent = String(rawValue);
        }

        // Enable inline editing if we have a PK column.
        if (editable && pkCol && col !== pkCol) {
          td.classList.add("editable");
          td.title = "Double-click to edit";
          td.addEventListener("dblclick", () => {
            this._makeEditable(td, col, pkCol, row[pkCol], rawValue);
          });
        }

        tr.appendChild(td);
      }
      tbody.appendChild(tr);
    }
    table.appendChild(tbody);

    this.resultArea.innerHTML = "";
    // Row count info bar.
    const info = document.createElement("div");
    info.className = "result-message";
    info.textContent = `${result.row_count} row(s) returned`;
    this.resultArea.appendChild(info);
    this.resultArea.appendChild(table);
  }

  // ── Inline cell editing ────────────────────────────────────────────────────

  private _makeEditable(
    td: HTMLTableCellElement,
    column: string,
    pkColumn: string,
    pkValue: string | number | null,
    currentValue: string | number | null
  ): void {
    if (!this.currentTable || pkValue === null) return;

    const original = td.textContent ?? "";
    const input = document.createElement("input");
    input.className = "cell-input";
    input.value = td.classList.contains("null-val") ? "" : original;
    td.innerHTML = "";
    td.appendChild(input);
    input.focus();
    input.select();

    const commit = async (): Promise<void> => {
      const newValue = input.value === "" ? null : input.value;
      if (newValue === currentValue || (newValue === null && currentValue === null)) {
        td.textContent = original;
        if (currentValue === null) td.className = "null-val";
        return;
      }

      try {
        await dbService.updateCell(
          this.currentTable!,
          pkColumn,
          pkValue as string | number,
          column,
          newValue
        );
        td.textContent = newValue ?? "NULL";
        td.className = newValue === null ? "null-val editable" : "editable";
        td.title = "Double-click to edit";
        showToast("Cell updated", "success");
      } catch (err) {
        td.textContent = original;
        showToast(`Update failed: ${String(err)}`, "error");
      }
    };

    input.addEventListener("blur", commit);
    input.addEventListener("keydown", (e) => {
      if (e.key === "Enter") { e.preventDefault(); input.blur(); }
      if (e.key === "Escape") {
        td.textContent = original;
        if (currentValue === null) td.className = "null-val editable";
      }
    });
  }

  // ── Helpers ────────────────────────────────────────────────────────────────

  private _showMessage(msg: string, type: "success" | "error" | "info"): void {
    this.resultArea.innerHTML = `<div class="result-message ${type}">${this._esc(msg)}</div>`;
  }

  private _esc(s: string): string {
    return s
      .replace(/&/g, "&amp;")
      .replace(/</g, "&lt;")
      .replace(/>/g, "&gt;")
      .replace(/"/g, "&quot;");
  }
}
