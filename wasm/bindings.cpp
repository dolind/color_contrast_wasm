#include <emscripten/bind.h>

#include "cell.hpp"


using namespace color_contrast;

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include "search.hpp"
using namespace emscripten;


EMSCRIPTEN_BINDINGS (color_grid_module) {
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
