#include <vector>
#include <cstdint>
#include <algorithm>
#include <random>
#include <emscripten/bind.h>

#include "cell.hpp"
#include "grid.hpp"
#include "brute_force.hpp"
#include "engine.hpp"
using namespace color_contrast;

// Same semantics as native main()
GridResult compute_grid(int dim, int iterations)
{
    AlgorithmConfig cfg;
    cfg.dim = dim;
    cfg.max_iterations = iterations;

    Engine e(std::make_unique<BruteForce>());
    Grid best = e.compute(cfg);

    return best.toResult();
}

#include <emscripten/bind.h>
#include <emscripten/val.h>

using namespace emscripten;

val compute_grid_js(int dim, int iters)
{
    auto res = compute_grid(dim, iters); // This returns GridResult

    val jsResult = val::object();
    jsResult.set("score", res.score);

    val jsArray = val::array();
    for (size_t i = 0; i < res.colors.size(); ++i) {
        const auto& cl = res.colors[i];

        val jsColor = val::object();
        jsColor.set("name", cl.name);

        // ✅ proper array construction in embind
        val rgb = val::array();
        rgb.set(0, cl.rgbValue.r);
        rgb.set(1, cl.rgbValue.g);
        rgb.set(2, cl.rgbValue.b);

        jsColor.set("rgb", rgb);
        jsArray.set(i, jsColor);
    }

    jsResult.set("colors", jsArray);
    return jsResult;
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

    emscripten::function("compute_grid", &compute_grid);
}
