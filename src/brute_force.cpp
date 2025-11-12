#include "brute_force.hpp"

#include <stdexcept>

#include "improved_grid.hpp"

namespace color_contrast {
    GridResult BruteForce::run() {
        for (int i = 0; i < cfg_.max_iterations; ++i)
            step();

        return bestGrid->toResult();
    }

    GridResult BruteForce::step() {
        std::unique_ptr<BaseGrid> trial;
        switch (cfg_.grid_type) {
            case GridType::Simple:
                trial = std::make_unique<Grid>(cfg_.dim, cfg_.uniformColorDistribution);
                break;
            case GridType::Improved:
                trial = std::make_unique<GridImprove>(cfg_.dim, cfg_.uniformColorDistribution);
                break;
            default:
                throw std::invalid_argument("Unknown grid type");
        }

        if (double newScore = trial->getScore(); newScore > bestScore) {
            bestScore = newScore;
            bestGrid = std::move(trial);
        }

        stepsDone++;
        bestGrid->setIterations(stepsDone);
        return bestGrid->toResult();
    }
}
