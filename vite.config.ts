import { defineConfig } from "vite";
import monacoEditorPlugin from "vite-plugin-monaco-editor";

// https://vitejs.dev/config/
export default defineConfig({
  // Source root is the project root (index.html lives at root level).
  root: ".",
  // Output directory for `npm run build`.
  build: {
    outDir: "dist",
    emptyOutDir: true,
  },
  plugins: [
    // Bundle Monaco Editor workers and language services automatically.
    // This is critical for offline operation — no CDN references.
    monacoEditorPlugin.default({
      languageWorkers: ["editorWorkerService", "css", "html", "json", "typescript"],
    }),
  ],
  // Prevent Vite from clearing the screen on each HMR update (friendlier in Tauri dev mode).
  clearScreen: false,
  server: {
    port: 5173,
    // Tauri expects a constant port; fail fast if something is already using it.
    strictPort: true,
    // Required so Tauri can communicate with the dev server from any origin.
    host: "localhost",
  },
  // Let Vite know about Tauri's platform-specific environment variables.
  envPrefix: ["VITE_", "TAURI_"],
  resolve: {
    alias: {
      "@": "/src",
    },
  },
});
