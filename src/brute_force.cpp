#include "brute_force.hpp"

namespace color_contrast {


Grid BruteForce::run(const AlgorithmConfig& cfg)
{
    // Legacy full compute (still works if called normally)
    for (int i = 0; i < cfg.max_iterations; ++i)
        step(cfg);

    return bestGrid;
}

Grid BruteForce::step(const AlgorithmConfig& cfg)
{
    Grid trial(cfg.dim, cfg.dim);

    double newScore = trial.getScore();
    if (newScore > bestScore) {
        bestScore = newScore;
        bestGrid = trial;
    }

    stepsDone++;
   bestGrid.setIterations(stepsDone);
    return bestGrid;
}
}