#include "simulated_annealing.hpp"
#include <cmath>

namespace color_contrast {

SimulatedAnnealing::SimulatedAnnealing(const AlgorithmConfig& cfg)
    : Algorithm(cfg),
      rng(std::random_device{}())
{
    currentGrid = std::make_unique<GridImprove>(cfg.dim, cfg.uniformColorDistribution);
    bestGrid    = std::make_unique<GridImprove>(*currentGrid);

    bestScore = bestGrid->getScore();
    stepsDone = 0;

    // Read temperature schedule from config
    T0 = cfg.startTemp;       // e.g. 1.0
    Tend = cfg.endTemp;       // e.g. 0.001
    cooling = std::pow(Tend / T0, 1.0 / cfg.max_iterations);;    // e.g. 0.9995
    T = T0;
}

GridResult SimulatedAnnealing::run() {
    for (int i = 0; i < cfg_.max_iterations; ++i)
        step();

    return bestGrid->toResult();
}

GridResult SimulatedAnnealing::step() {
    ++stepsDone;

    GridImprove* g = currentGrid.get();
    double oldScore = g->getScore();

    bool accepted = tryMove(g, oldScore);
    if (accepted) {
        double newScore = g->getScore();
        if (newScore > bestScore) {
            bestScore = newScore;
            *bestGrid = *currentGrid;
        }
    }

    // Cool temperature
    T = std::max(T * cooling, Tend);

    bestGrid->setIterations(stepsDone);
    return bestGrid->toResult();
}

bool SimulatedAnnealing::tryMove(GridImprove* g, double oldScore) {
    std::uniform_real_distribution<double> choose(0.0, 1.0);

    // Mostly swapping; recolor occasionally
    bool useSwap = choose(rng) < 0.9;

    return useSwap ? trySwap(g, oldScore)
                   : tryRecolor(g, oldScore);
}

bool SimulatedAnnealing::accept(double delta) {
    // If improvement → always accept
    if (delta > 0)
        return true;

    // Otherwise accept with probability exp(delta / T)
    double prob = std::exp(delta / T); // delta < 0
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    return dist(rng) < prob;
}

//------------------------------------------------------------
// Swap move
//------------------------------------------------------------
bool SimulatedAnnealing::trySwap(GridImprove* g, double oldScore) {
    int total = g->width() * g->height();
    if (total <= 1) return false;

    std::uniform_int_distribution<int> dist(0, total - 1);

    int a = dist(rng);
    int b = dist(rng);
    if (a == b) return false;

    // Apply move
    g->swapCells(a, b);
    g->updateDelta(a, b);

    double newScore = g->getScore();
    double delta = newScore - oldScore;

    if (accept(delta)) {
        return true; // keep change
    }

    // Undo move
    g->swapCells(a, b);
    g->restoreDelta(a, b);
    return false;
}

//------------------------------------------------------------
// Recolor move
//------------------------------------------------------------
bool SimulatedAnnealing::tryRecolor(GridImprove* g, double oldScore) {
    int total = g->width() * g->height();
    if (total == 0) return false;

    std::uniform_int_distribution<int> cell(0, total - 1);
    std::uniform_int_distribution<int> col(0, COLORS_AVAILABLE - 1);

    int idx = cell(rng);
    int oldC = g->getColorIndex(idx);
    int newC = col(rng);

    if (oldC == newC) return false;

    const int targetDistinct = std::min(total, COLORS_AVAILABLE);


    // Apply move
    g->recolorCell(idx, newC);
    g->updateDelta(idx);

    // Reject recolors that reduce distinct colors
    // Should be improved to limit color frequencies
    if (g->countDistinctColors() < targetDistinct) {
        // Undo recolor
        g->recolorCell(idx, oldC);
        g->restoreDelta(idx);
        return false;
    }

    double newScore = g->getScore();
    double delta = newScore - oldScore;

    if (accept(delta)) {
        return true;
    }

    // Undo move
    g->recolorCell(idx, oldC);
    g->restoreDelta(idx);
    return false;
}

} // namespace color_contrast
