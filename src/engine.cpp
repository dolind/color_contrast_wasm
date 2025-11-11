#include "engine.hpp"
namespace color_contrast {


Engine::Engine(std::unique_ptr<Algorithm> algo, AlgorithmConfig cfg)
    : algorithm_(std::move(algo)), cfg_(cfg) {}

Grid Engine::compute(const AlgorithmConfig& cfg) {
    return algorithm_->run(cfg);
}

Grid Engine::step() {
    return algorithm_->step(cfg_);
}
}