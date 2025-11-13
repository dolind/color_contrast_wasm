#pragma once
#include <random>
#include "algorithm.hpp"
#include "improved_grid.hpp"
namespace color_contrast {
    class HillClimb final : public Algorithm {
    public:
        explicit HillClimb(const AlgorithmConfig& cfg);

        GridResult run() override;
        GridResult step() override;

    private:
        std::mt19937 rng;
        int stuck = 0;

        // Helpers
        bool trySwap(GridImprove* g);
        bool tryRecolor(GridImprove* g);
    };
}