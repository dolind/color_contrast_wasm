#pragma once
#include "algorithm.hpp"
namespace color_contrast {


class BruteForce : public Algorithm {

public:
    explicit BruteForce(const AlgorithmConfig& cfg) : Algorithm(cfg){}
    GridResult step() override;
    GridResult run() override;

};
}