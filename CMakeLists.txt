cmake_minimum_required(VERSION 3.8.1)
project(color_contrast VERSION 1.0.0 LANGUAGES CXX)

# ======================================================
# Compile settings for all targets
# ======================================================

# Use C++17 (OpenCV-era placeholder used cxx_std_11, but we standardize here)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Always enforce -O2 and make builds portable (no -march=native)
add_compile_options(
    "$<$<CONFIG:RELEASE>:-O2;-Wall;-Wextra;-DNDEBUG>"
    "$<$<CONFIG:DEBUG>:-g3;-Wall;-Wextra>"
)

# Remove dangerous flags if inherited from environment
string(REPLACE "-march=native" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
string(REPLACE "-march=native" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# ======================================================
# Library: cpp_engine
# ======================================================

add_library(color_contrast_engine
    src/grid.cpp
    src/cell.cpp
    src/engine.cpp
    src/brute_force.cpp
)

target_include_directories(color_contrast_engine
    PUBLIC
        include
)

# ======================================================
# Optional executable (you may delete if unnecessary)
# ======================================================

add_executable(color_contrast
    src/main.cpp
)

# alias
add_executable(my::color_contrast ALIAS color_contrast)

target_link_libraries(color_contrast
    PRIVATE
        color_contrast_engine
)


# ======================================================
# Wasm target  (only active when building via emcmake)
# ======================================================


if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Emscripten")

    message(STATUS ">>> Building WebAssembly target")

    add_executable(colorgrid_wasm
        wasm/bindings.cpp     # thin embind wrapper
        src/grid.cpp
        src/cell.cpp
        src/engine.cpp
        src/brute_force.cpp
    )

    target_include_directories(colorgrid_wasm PUBLIC include)

    # Required wasm options
    target_link_options(colorgrid_wasm PRIVATE
        "-sALLOW_MEMORY_GROWTH=1"
        "-sMODULARIZE=1"
        "-sEXPORT_NAME=createColorGridModule"
        "-sENVIRONMENT=web"
        "-sWASM=1"
        "-sEMBIND"
    )

    # Output to predictable folder: build_wasm/wasm_out/
    set_target_properties(colorgrid_wasm PROPERTIES
        OUTPUT_NAME "colorgrid"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/wasm_out"
    )

endif()