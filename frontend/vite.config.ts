import { defineConfig } from 'vite';

export default defineConfig({
  base: '/color_contrast_wasm/',
  server: {
    port: 5173,
    headers: {
      "Cross-Origin-Opener-Policy": "same-origin",
      "Cross-Origin-Embedder-Policy": "require-corp"
    }
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
