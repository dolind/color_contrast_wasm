cmake_minimum_required(VERSION 3.28.3)
project(color_contrast VERSION 1.0.0 LANGUAGES CXX)

# ======================================================
# Compile settings for all targets
# ======================================================

# Use C++17 (OpenCV-era placeholder used cxx_std_11, but we standardize here)
set(CMAKE_CXX_STANDARD 20)
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
        src/brute_force.cpp
        src/search.cpp
        src/improved_grid.cpp
        src/contrast_table.cpp

)

target_include_directories(color_contrast_engine
        PUBLIC
        include
)

# ======================================================
# Wasm target  (only active when building via emcmake)
# ======================================================


if (EMSCRIPTEN)
    message(STATUS ">>> Building WebAssembly target")

    add_executable(color_contrast_wasm
            wasm/bindings.cpp
    )

    target_include_directories(color_contrast_wasm
            PUBLIC
            include
    )

    target_link_libraries(color_contrast_wasm
            PRIVATE
            color_contrast_engine
    )

    # Required wasm options
    target_link_options(color_contrast_wasm PRIVATE
            "-sMODULARIZE=1"
            "-sEXPORT_ES6=1"
            "-sEXPORT_NAME=createColorGridModule"
            "-sENVIRONMENT=web"
            "-sWASM=1"
            "-sALLOW_MEMORY_GROWTH=1"
            "-sSINGLE_FILE=1"
            "--bind"
            "-sNO_EXIT_RUNTIME=1"
    )


else ()

    # ======================================================
    # Optional executable (you may delete if unnecessary)
    # ======================================================
    message(STATUS ">>> Building Native target")
    add_executable(color_contrast
            src/main.cpp
    )

    # alias
    add_executable(my::color_contrast ALIAS color_contrast)

    target_link_libraries(color_contrast
            PRIVATE
            color_contrast_engine
    )


endif ()