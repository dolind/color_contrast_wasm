#include <vector>
#include <cstdint>
#include <algorithm>
#include <random>
#include <emscripten/bind.h>

#include "color_cell.hpp"
#include "color_grid.hpp"

using namespace color_contrast;

// Same semantics as native main()
GridResult wasm_compute_grid(int dim, int iterations)
{
    AlgorithmConfig cfg;
    cfg.dim = dim;              // let algorithm choose or expose dim to JS
    cfg.max_iterations = iterations;

    Engine e(std::make_unique<BruteForce>());
    Grid best = e.compute(cfg);      // <-- identical logic to native main()

    // Let the grid convert itself to a JS-serializable result
    return best.toResult();
}

EMSCRIPTEN_BINDINGS(color_grid_module) {

    emscripten::value_object<RGB>("RGB")
        .field("r", &RGB::r)
        .field("g", &RGB::g)
        .field("b", &RGB::b);

    emscripten::value_object<COilColor>("COilColor")
        .field("name", &COilColor::name)
        .field("rgbValue", &COilColor::rgbValue);

    emscripten::register_vector<COilColor>("vector<COilColor>");

    emscripten::value_object<GridResult>("GridResult")
        .field("colors", &GridResult::colors)
        .field("score", &GridResult::score);

    emscripten::function("compute_grid", &wasm_compute_grid);
}
