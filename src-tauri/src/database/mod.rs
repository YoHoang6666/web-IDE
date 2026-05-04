// database/mod.rs
// SQLite wrapper used by the Tauri command handlers.
// `DbManager` holds one open connection at a time; the frontend opens a
// database file via `db_open` and then issues queries / statements against it.

use anyhow::{anyhow, Result};
use rusqlite::{Connection, Row, types::ValueRef};
use serde::{Deserialize, Serialize};

// ─────────────────────────────────────────────────────────────────────────────
// Public types
// ─────────────────────────────────────────────────────────────────────────────

/// One row from a SELECT result: a map from column name → JSON value.
pub type RowData = std::collections::HashMap<String, serde_json::Value>;

/// The full result of a SELECT query.
#[derive(Debug, Serialize, Deserialize)]
pub struct QueryResult {
    /// Ordered list of column names.
    pub columns: Vec<String>,
    /// All rows returned by the query.
    pub rows: Vec<RowData>,
    /// How many rows were returned.
    pub row_count: usize,
}

/// Information about a single column in a table (from PRAGMA table_info).
#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct TableInfo {
    /// Column index (0-based).
    pub cid: i64,
    pub name: String,
    /// SQLite type affinity string (e.g. "TEXT", "INTEGER").
    pub col_type: String,
    /// Whether the column disallows NULL.
    pub not_null: bool,
    /// Default value expression (empty string if none).
    pub default_value: String,
    /// Whether the column is part of the primary key.
    pub primary_key: bool,
}

// ─────────────────────────────────────────────────────────────────────────────
// DbManager
// ─────────────────────────────────────────────────────────────────────────────

/// Manages a single SQLite connection.
/// Methods are called with `&self` / `&mut self` from within the `Mutex` guard
/// held by `AppState`.
pub struct DbManager {
    /// The active connection, or `None` if no database has been opened yet.
    connection: Option<Connection>,
}

impl DbManager {
    /// Create a new, idle manager (no database open).
    pub fn new() -> Self {
        Self { connection: None }
    }

    /// Open (or create) an SQLite file at `path`.
    /// Enables WAL journal mode for better concurrent read performance.
    pub fn open(&mut self, path: &str) -> Result<()> {
        let conn = Connection::open(path)?;
        // Use WAL for better performance in concurrent read scenarios.
        conn.execute_batch("PRAGMA journal_mode=WAL; PRAGMA foreign_keys=ON;")?;
        self.connection = Some(conn);
        Ok(())
    }

    /// Return a reference to the active connection, or an error if none is open.
    fn conn(&self) -> Result<&Connection> {
        self.connection
            .as_ref()
            .ok_or_else(|| anyhow!("No database is open. Call db_open first."))
    }

    /// Execute a non-SELECT SQL statement and return rows-affected count.
    pub fn execute(&self, sql: &str) -> Result<usize> {
        let conn = self.conn()?;
        let affected = conn.execute(sql, [])?;
        Ok(affected)
    }

    /// Execute a SELECT query and return a structured result.
    pub fn query(&self, sql: &str) -> Result<QueryResult> {
        let conn = self.conn()?;
        let mut stmt = conn.prepare(sql)?;

        // Collect column names from the prepared statement.
        let columns: Vec<String> = stmt
            .column_names()
            .iter()
            .map(|s| s.to_string())
            .collect();

        // Map each SQLite value to a `serde_json::Value` for easy serialisation.
        let col_count = columns.len();
        let rows_iter = stmt.query_map([], |row: &Row| {
            let mut map = RowData::new();
            for i in 0..col_count {
                let col_name = row.as_ref().column_name(i).unwrap_or("").to_string();
                let value = match row.get_ref_unwrap(i) {
                    ValueRef::Null => serde_json::Value::Null,
                    ValueRef::Integer(n) => serde_json::Value::Number(n.into()),
                    ValueRef::Real(f) => {
                        serde_json::Number::from_f64(f)
                            .map(serde_json::Value::Number)
                            .unwrap_or(serde_json::Value::Null)
                    }
                    ValueRef::Text(t) => {
                        serde_json::Value::String(
                            std::str::from_utf8(t).unwrap_or("").to_string(),
                        )
                    }
                    ValueRef::Blob(b) => {
                        // Return blobs as a hex string.
                        serde_json::Value::String(hex::encode(b))
                    }
                };
                map.insert(col_name, value);
            }
            Ok(map)
        })?;

        let mut rows: Vec<RowData> = Vec::new();
        for row in rows_iter {
            rows.push(row?);
        }
        let row_count = rows.len();
        Ok(QueryResult {
            columns,
            rows,
            row_count,
        })
    }

    /// Return names of all user-defined tables (excludes sqlite_* system tables).
    pub fn list_tables(&self) -> Result<Vec<String>> {
        let conn = self.conn()?;
        let mut stmt = conn.prepare(
            "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%' ORDER BY name",
        )?;
        let names: Vec<String> = stmt
            .query_map([], |row| row.get::<_, String>(0))?
            .filter_map(|r| r.ok())
            .collect();
        Ok(names)
    }

    /// Return column definitions for `table` via PRAGMA table_info.
    pub fn table_schema(&self, table: &str) -> Result<Vec<TableInfo>> {
        let conn = self.conn()?;
        // Use a parameterised query to avoid SQL-injection of the table name.
        let sql = format!("PRAGMA table_info(\"{}\")", table.replace('"', "\"\""));
        let mut stmt = conn.prepare(&sql)?;
        let infos: Vec<TableInfo> = stmt
            .query_map([], |row| {
                Ok(TableInfo {
                    cid: row.get(0)?,
                    name: row.get(1)?,
                    col_type: row.get(2)?,
                    not_null: row.get::<_, i32>(3)? != 0,
                    default_value: row.get::<_, Option<String>>(4)?.unwrap_or_default(),
                    primary_key: row.get::<_, i32>(5)? != 0,
                })
            })?
            .filter_map(|r| r.ok())
            .collect();
        Ok(infos)
    }
}

// Make DbManager sendable across threads (required by Tauri's State).
// Safety: we never share the Connection across threads without the Mutex.
unsafe impl Send for DbManager {}
