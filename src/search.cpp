#include <iostream>
#include <ostream>
#include <stdexcept>
#include <unordered_map>

#include "algorithm.hpp"
#include "brute_force.hpp"
#include "contrast_table.hpp"


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

        // case AlgorithmType::BeamSearch:
        //     algos[id] = std::make_unique<BeamSearch>(std::move(cfg));
        //     break;
        //
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

GridResult step_search(int id) {
    return algos[id]->step();
}


