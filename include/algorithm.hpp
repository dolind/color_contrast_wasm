#pragma once
#include <memory>

#include "grid.hpp"

enum class AlgorithmType {
    BruteForce=0,
    HillClimbing=1,
    SimulatedAnnealing=3,
    BeamSearch=2,

    Genetic=4
};

enum class GridType {
    Simple,
    Improved,
    Random
};

struct AlgorithmConfig {
    int max_iterations = 5000;
    int dim = 6;
    int beam_width = 0;
    bool uniformColorDistribution = false;
    AlgorithmType algo = AlgorithmType::BruteForce;
    GridType grid_type = GridType::Simple;

    // Parameters for simulated annealing
    double startTemp = 1.0;
    double endTemp   = 0.001;
    double cooling   = 0.9995;
};

namespace color_contrast {
    class Algorithm {
    public:
        explicit Algorithm(const AlgorithmConfig &cfg) : cfg_(cfg) {
        }

        virtual ~Algorithm() = default;

        // Compute the best grid from a given starting point
        virtual GridResult run() = 0;

        virtual GridResult step() = 0;

        int getSteps() const { return stepsDone; };
        BaseGrid* getBestGrid() const { return bestGrid.get(); }

    protected:
        std::unique_ptr<BaseGrid> bestGrid;
        double bestScore = -1.0;
        int stepsDone = 0;
        AlgorithmConfig cfg_;
    };
}
