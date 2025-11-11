#include "engine.hpp"
#include "brute_force.hpp"
#include <iostream>

int main() {
    AlgorithmConfig cfg{50000};

    color_contrast::Engine e(std::make_unique<color_contrast::BruteForce>());
    auto best = e.compute(cfg);
    best.printToConsole();
    std::cout << "done\n";
}
