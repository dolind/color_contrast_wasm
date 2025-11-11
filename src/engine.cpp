#include "engine.hpp"
namespace color_contrast {


Engine::Engine(std::unique_ptr<Algorithm> algo)
    : algorithm_(std::move(algo)) {}

Grid Engine::compute(const AlgorithmConfig& cfg) {
    return algorithm_->run(cfg);
}
}