
#include <memory>

#include <emscripten/bind.h>

#include "cell.hpp"
#include "grid.hpp"
#include "brute_force.hpp"
using namespace color_contrast;


std::unordered_map<int, std::unique_ptr<Algorithm>> algos;


void start_search(int id, int dim, int max_iters)
{
    AlgorithmConfig cfg;
    cfg.dim = dim;
    cfg.max_iterations = max_iters;

    algos[id] = std::make_unique<BruteForce> (cfg);
    }

GridResult step_search(int id) {
    return algos[id]->step();
}

#include <emscripten/bind.h>
#include <emscripten/val.h>

using namespace emscripten;



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
        .field("score", &GridResult::score)
        .field("iterations", &GridResult::iterations);


    emscripten::function("start_search", &start_search);
    emscripten::function("step_search", &step_search);

}
