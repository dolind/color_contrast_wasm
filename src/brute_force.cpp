#include "brute_force.hpp"

namespace color_contrast {


Grid BruteForce::run(const AlgorithmConfig& cfg) {


    color_contrast::Grid bestGrid;
    double bestScore{0};
    for (int i = 0; i < cfg.max_iterations; ++i) {
        color_contrast::Grid trial(cfg.dim, cfg.dim);

        double newScore = trial.getScore();
        if (newScore > bestScore) {
            bestScore = newScore;
            bestGrid = trial;
        }
    }

    return bestGrid;
}
}