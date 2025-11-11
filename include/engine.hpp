#pragma once
#include <memory>
#include "grid.hpp"
#include "algorithm.hpp"
namespace color_contrast {


class Engine {
public:
    explicit Engine(std::unique_ptr<Algorithm> algo, AlgorithmConfig cfg);
    Grid step();
    Grid compute(const AlgorithmConfig& cfg);

private:
    std::unique_ptr<Algorithm> algorithm_;
    AlgorithmConfig cfg_;
};
}