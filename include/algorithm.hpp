#pragma once
#include <memory>

#include "grid.hpp"

struct AlgorithmConfig {
    int max_iterations = 5000;
    int dim = 6;
    int beam_width = 0;
};
namespace color_contrast {


class Algorithm {
public:
    explicit Algorithm(const AlgorithmConfig& cfg): cfg_(cfg){}

    virtual ~Algorithm() = default;

    // Compute best grid from given starting point
    virtual GridResult run() = 0;
    virtual GridResult step() = 0;

    int getSteps() const{return stepsDone;};
protected:
    std::unique_ptr<BaseGrid> bestGrid;
    double bestScore = -1.0;
    int stepsDone = 0;
    AlgorithmConfig cfg_;
};
}