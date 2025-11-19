#include <iostream>
#include <memory>

#include "algorithm.hpp"
#include "search.hpp"

using namespace color_contrast;

int main() {
    try {
        int id = 0;
        int dim = 3;
        int max_iters = 200000;
        int grid_type = static_cast<int>(GridType::Improved);
        int algo_type = static_cast<int>(AlgorithmType::HillClimbing);
        int uniformColorDistribution = 0;

        start_search(id, dim, max_iters, grid_type, algo_type, uniformColorDistribution);

        StepInfo info{};
        for (int i = 0; i < max_iters; ++i) {
            info = step_search_info(id);
        }

        std::cout << "Best score: " << info.score << "\n";
        std::cout << "Iterations: " << info.iterations << "\n";
        std::cout << "Done.\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}