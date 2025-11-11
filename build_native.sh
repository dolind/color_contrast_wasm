#!/usr/bin/env bash

set -e

echo "=== Building native C++ binary ==="

# create build folder if missing
mkdir -p build_native
cd build_native

# configure + build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE
cmake --build .

echo ""
echo "✅ Native build done."
echo "👉 Binary: build_native/color_contrast"
