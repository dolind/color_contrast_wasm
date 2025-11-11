#include <vector>
#include <cstdint>
#include <algorithm>
#include <random>
#include <emscripten/bind.h>

#include "color_cell.hpp"
#include "color_grid.hpp"

// Run 'iterations' random grids of size dim x dim,
// return the best grid's colors as [r,g,b, r,g,b, ...] row-major.
std::vector<std::uint8_t> generate_best_grid_rgb(int dim, int iterations)
{
    using namespace color_contrast;

    double bestScore = -1.0;
    CGrid best(dim, dim);

    for (int i = 0; i < iterations; ++i) {
        CGrid g(dim, dim);
        double s = g.getScore();
        if (s > bestScore) {
            bestScore = s;
            best = g; // copy
        }
    }

    // Flatten to RGB bytes, row-major
    std::vector<std::uint8_t> out;
    out.reserve(dim * dim * 3);

    for (int y = 0; y < best.height(); ++y) {
        for (int x = 0; x < best.width(); ++x) {
            RGB c = best.at(x, y).getRGBColor();
            out.push_back(c.r);
            out.push_back(c.g);
            out.push_back(c.b);
        }
    }
    return out;
}

EMSCRIPTEN_BINDINGS(color_grid_module) {
    emscripten::function("generate_best_grid_rgb", &generate_best_grid_rgb);
}
