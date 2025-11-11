#pragma once
#include "algorithm.hpp"
namespace color_contrast {


class BruteForce : public Algorithm {
public:
    Grid step(const AlgorithmConfig& cfg) override;
    Grid run(const AlgorithmConfig& cfg) override;

};
}