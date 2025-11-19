#include "hill_climb.hpp"
#include <algorithm>
#include <iostream>

using namespace color_contrast;

HillClimb::HillClimb(const AlgorithmConfig& cfg)
    : Algorithm(cfg), rng(std::random_device{}())
{
    // Initialize first grid
    bestGrid = std::make_unique<GridImprove>(cfg.dim, cfg.uniformColorDistribution);
    bestScore = bestGrid->getScore();
}

GridResult HillClimb::run() {
    for (int i = 0; i < cfg_.max_iterations; ++i)
        step();

    return bestGrid->toResult();
}

GridResult HillClimb::step() {
    ++stepsDone;
    auto* g = dynamic_cast<GridImprove*>(bestGrid.get());
    bool improved = false;

    // Try swap
    if (trySwap(g))
        improved = true;

    // Record stuck status
    if (!improved)
        stuck++;
    else
        stuck = 0;

    // Recolor as a restart mechanism
    if (stuck > 300) {
        stuck = 0;
        tryRecolor(g);
    }

    bestGrid->setIterations(stepsDone);
    return bestGrid->toResult();
}

bool HillClimb::trySwap(GridImprove* g) {
    int total = g->width() * g->height();
    if (total <= 1) return false;

    std::uniform_int_distribution<int> dist(0, total - 1);

    int a = dist(rng);
    int b = dist(rng);
    if (a == b) return false;

    double oldScore = g->getScore();

    g->swapCells(a, b);
    g->updateDelta(a, b);

    double newScore = g->getScore();
    if (newScore > oldScore) {
        bestScore = newScore;
        return true;
    }

    // Undo
    g->swapCells(a, b);
    g->restoreDelta(a, b);
    return false;
}

bool HillClimb::tryRecolor(GridImprove* g) {
    int total = g->width() * g->height();
    if (total == 0) return false;

    std::uniform_int_distribution<int> dCell(0, total - 1);
    std::uniform_int_distribution<int> dCol(1, COLORS_AVAILABLE - 1);

    int idx = dCell(rng);
    int oldColor = g->getColorIndex(idx);
    int newColor = dCol(rng);

    if (newColor == oldColor) return false;

    const int targetDistinct =
        std::min(total, COLORS_AVAILABLE);

    double oldScore = g->getScore();

    g->recolorCell(idx, newColor);
    g->updateDelta(idx);

    // Reject recolors that reduce distinct colors
    if (g->countDistinctColors() < targetDistinct) {
        // Undo recolor
        g->recolorCell(idx, oldColor);
        g->restoreDelta(idx);
        return false;
    }

    if (g->getScore() >= oldScore) {
        bestScore = g->getScore();
        return true;
    }

    // Undo
    g->recolorCell(idx, oldColor);
    g->restoreDelta(idx);
    return false;
}
