#include "search.hpp"

#include <iostream>
#include <ostream>
#include <stdexcept>
#include <unordered_map>

#include "algorithm.hpp"
#include "brute_force.hpp"
#include "contrast_table.hpp"
#include "hill_climb.hpp"


using namespace color_contrast;

std::unordered_map<int, std::unique_ptr<Algorithm> > algos;


void start_search(int id, int dim, int max_iters, int grid_type, int algo_type, int uniformColorDistribution) {
    auto gtype = static_cast<GridType>(grid_type);


    auto atype = static_cast<AlgorithmType>(algo_type);

    AlgorithmConfig cfg;
    cfg.dim = dim;
    cfg.max_iterations = max_iters;
    cfg.grid_type = gtype;
    cfg.algo = atype;
    cfg.uniformColorDistribution = uniformColorDistribution;

    buildContrastTable();

    switch (atype) {
        case AlgorithmType::BruteForce:
            algos[id] = std::make_unique<BruteForce>(cfg);
            break;

        case AlgorithmType::HillClimbing:
            algos[id] = std::make_unique<HillClimb>(cfg);

            break;

        // case AlgorithmType::SimulatedAnnealing:
        //     algos[id] = std::make_unique<SimulatedAnnealing>(std::move(cfg));
        //     break;
        //
        // case AlgorithmType::Genetic:
        //     algos[id] = std::make_unique<GeneticAlgorithm>(std::move(cfg));
        //     break;

        default:
            throw std::invalid_argument("Unknown algorithm type");
    }
}
// TODO: this produces a memory leak as wasm memory will grow due to copy.
// Must be done using
StepInfo step_search_info(int id) {
    auto& algo = *algos.at(id);
    auto res = algo.step();   // this modifies algo.bestGrid internally

    StepInfo info;
    info.score = res.score;
    info.iterations = res.iterations;
    return info;
}

void export_grid_rgb(int id, std::uint8_t* out, int max_len) {
    auto& algo = *algos.at(id);
    auto* grid = algo.getBestGrid();
    if (!grid) return;

    const int w = grid->width();
    const int h = grid->height();
    const int needed = w * h * 3;

    if (max_len < needed) return; // guard

    int idx = 0;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const auto& cell = grid->at(x, y);
            RGB rgb = cell.getRGBColor();
            out[idx++] = rgb.r;
            out[idx++] = rgb.g;
            out[idx++] = rgb.b;
        }
    }
}