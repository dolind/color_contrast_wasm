#pragma once
#include "grid.hpp"

struct AlgorithmConfig {
    int max_iterations = 5000;
    int dim = 6;
    int beam_width = 0;
};
namespace color_contrast {


class Algorithm {
public:
    virtual ~Algorithm() = default;

    // Compute best grid from given starting point
    virtual Grid run(const AlgorithmConfig& cfg) = 0;
};
}