#!/usr/bin/env bash
set -e

# where we keep external dependencies
EMSDK_DIR="external/emsdk"

echo "=== Checking for Emscripten SDK ==="

if [ ! -d "$EMSDK_DIR" ]; then
    echo "📦 emsdk not found, cloning..."
    mkdir -p external
    git clone https://github.com/emscripten-core/emsdk.git "$EMSDK_DIR"
else
    echo "✅ emsdk already present."
fi

cd "$EMSDK_DIR"

echo "=== Installing latest SDK ==="
./emsdk install latest

echo "=== Activating SDK ==="
./emsdk activate latest

echo "=== Loading environment variables ==="
source ./emsdk_env.sh

echo ""
echo "✅ Emscripten is ready."
echo "➡ Try: emcc --version"
echo "➡ To build WASM: ./build_wasm.sh"
