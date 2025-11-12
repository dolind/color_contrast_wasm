#include "brute_force.hpp"

namespace color_contrast {


GridResult BruteForce::run()
{

    for (int i = 0; i < cfg_.max_iterations; ++i)
        step();

    return bestGrid->toResult();
}

GridResult BruteForce::step()
{
    std::unique_ptr<BaseGrid> trial = std::make_unique<Grid>(cfg_.dim, cfg_.dim);

    double newScore = trial->getScore();
    if (newScore > bestScore) {
        bestScore = newScore;
        bestGrid = std::move(trial);
    }

    stepsDone++;
    bestGrid->setIterations(stepsDone);
    return bestGrid->toResult();
}
}