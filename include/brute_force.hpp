#pragma once
#include "algorithm.hpp"
namespace color_contrast {


class BruteForce : public Algorithm {
public:
    Grid run(const AlgorithmConfig& cfg) override;
};
}