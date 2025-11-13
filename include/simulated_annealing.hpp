#pragma once
#include "algorithm.hpp"
#include "improved_grid.hpp"
#include <random>

namespace color_contrast {

    class SimulatedAnnealing : public Algorithm {
    public:
        explicit SimulatedAnnealing(const AlgorithmConfig& cfg);

        GridResult run() override;
        GridResult step() override;

    private:
        // RNG
        std::mt19937 rng;

        // Temperature schedule
        double T;       // current temperature
        double T0;      // initial temperature
        double Tend;    // minimum temperature (stopping)
        double cooling; // exponential cooling rate

        bool tryMove(GridImprove* g, double oldScore);
        bool trySwap(GridImprove* g, double oldScore);
        bool tryRecolor(GridImprove* g, double oldScore);

        bool accept(double delta); // simulated annealing acceptance rule
    };

} // namespace color_contrast
