#include <emscripten/bind.h>

#include "cell.hpp"


using namespace color_contrast;

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include "search.hpp"
using namespace emscripten;


EMSCRIPTEN_BINDINGS(my_module) {
        value_object<StepInfo>("StepInfo")
            .field("score", &StepInfo::score)
            .field("iterations", &StepInfo::iterations);

        function("start_search", &start_search);
        function("step_search_info", &step_search_info);

        function("export_grid_rgb",
            optional_override([](int id, uintptr_t ptr, int len) {
                auto* buf = reinterpret_cast<std::uint8_t*>(ptr);
                export_grid_rgb(id, buf, len);
            })
        );
}
