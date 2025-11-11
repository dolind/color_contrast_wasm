import { defineConfig } from 'vite';

export default defineConfig({
  server: {
    port: 5173,
  },

  // Important for Emscripten output:
  // We don't want Vite to transform `.wasm` and `.js` glue files
  publicDir: "public",

  assetsInclude: ["**/*.wasm"],

  build: {
    target: "esnext",        // required for top-level await (WASM loading)
    sourcemap: true,
  }
});
