/**
 * dbService.ts
 * Bridge between the TypeScript frontend and the Rust SQLite commands exposed
 * by Tauri.  Mirrors the DTOs in db_commands.rs.
 */

import { invoke } from "@tauri-apps/api/core";

// ─────────────────────────────────────────────────────────────────────────────
// Types (must match the Rust DTOs)
// ─────────────────────────────────────────────────────────────────────────────

export interface DatabaseInfo {
  /** Absolute path to the `.db` / `.sqlite` file. */
  path: string;
  /** Friendly display name (basename without extension). */
  name: string;
}

/** One row: column name → JSON value. */
export type RowData = Record<string, string | number | null>;

export interface QueryResult {
  /** Ordered column names. */
  columns: string[];
  /** All rows returned by the query. */
  rows: RowData[];
  /** Number of rows returned. */
  row_count: number;
}

export interface TableInfo {
  cid: number;
  name: string;
  col_type: string;
  not_null: boolean;
  default_value: string;
  primary_key: boolean;
}

// ─────────────────────────────────────────────────────────────────────────────
// DbService
// ─────────────────────────────────────────────────────────────────────────────

class DbService {
  /** Currently open database info, or null if none is open. */
  private _currentDatabase: DatabaseInfo | null = null;

  get currentDatabase(): DatabaseInfo | null {
    return this._currentDatabase;
  }

  /**
   * Open (or create) an SQLite database at `path`.
   * Subsequent query / execute calls operate on this database.
   */
  async openDatabase(path: string): Promise<DatabaseInfo> {
    const info = await invoke<DatabaseInfo>("db_open", { path });
    this._currentDatabase = info;
    return info;
  }

  /**
   * Execute a non-SELECT statement (INSERT / UPDATE / DELETE / CREATE / DROP …).
   * Returns the number of rows affected.
   */
  async execute(sql: string): Promise<number> {
    return invoke<number>("db_execute", { sql });
  }

  /**
   * Run a SELECT query and return a structured result.
   */
  async query(sql: string): Promise<QueryResult> {
    return invoke<QueryResult>("db_query", { sql });
  }

  /**
   * Return the names of all user-defined tables in the open database.
   */
  async listTables(): Promise<string[]> {
    return invoke<string[]>("db_list_tables");
  }

  /**
   * Return column definitions for a table.
   */
  async tableSchema(tableName: string): Promise<TableInfo[]> {
    return invoke<TableInfo[]>("db_table_schema", { tableName: tableName });
  }

  /**
   * Scan `directory` for `.db` / `.sqlite` files and return their info.
   */
  async listDatabases(directory: string): Promise<DatabaseInfo[]> {
    return invoke<DatabaseInfo[]>("db_list_databases", { directory });
  }

  // ── Query helpers ──────────────────────────────────────────────────────────

  /**
   * Fetch all rows from `tableName` with optional LIMIT / OFFSET pagination.
   */
  async browseTable(
    tableName: string,
    limit = 500,
    offset = 0
  ): Promise<QueryResult> {
    const sql = `SELECT * FROM "${this._escapeTableName(tableName)}" LIMIT ${limit} OFFSET ${offset}`;
    return this.query(sql);
  }

  /**
   * Update a single cell value in a table.
   * Assumes the table has a primary-key column `pkColumn` with value `pkValue`.
   */
  async updateCell(
    tableName: string,
    pkColumn: string,
    pkValue: string | number,
    column: string,
    newValue: string | number | null
  ): Promise<void> {
    const val =
      newValue === null
        ? "NULL"
        : typeof newValue === "number"
        ? String(newValue)
        : `'${String(newValue).replace(/'/g, "''")}'`;

    const pkVal =
      typeof pkValue === "number"
        ? String(pkValue)
        : `'${String(pkValue).replace(/'/g, "''")}'`;

    const sql = `UPDATE "${this._escapeTableName(tableName)}"
                 SET "${column}" = ${val}
                 WHERE "${pkColumn}" = ${pkVal}`;
    await this.execute(sql);
  }

  /** Escape table / column names by doubling internal double-quotes. */
  private _escapeTableName(name: string): string {
    return name.replace(/"/g, '""');
  }
}

/** Singleton instance shared throughout the application. */
export const dbService = new DbService();
