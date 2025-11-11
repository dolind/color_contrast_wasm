#!/usr/bin/env bash

set -e

if [ -z "$EMSDK" ]; then
  echo "❌ ERROR: Emscripten is not activated"
  echo "Run: source ~/emsdk/emsdk_env.sh"
  exit 1
fi

echo "=== Building WebAssembly target ==="

mkdir -p build_wasm
cd build_wasm

# configure using emcmake
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release

# run build
cmake --build .

echo ""
echo "✅ WASM build done."

echo ""
cp build_wasm/color_contrast.* frontend/public/