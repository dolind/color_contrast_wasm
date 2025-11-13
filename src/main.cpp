#include <iostream>
#include <memory>

#include "algorithm.hpp"
#include "search.hpp"

using namespace color_contrast;


int main() {
    try {
        int id = 0;
        int dim = 3;
        int max_iters = 50000;
        int grid_type = static_cast<int>(GridType::Improved);
        int algo_type = static_cast<int>(AlgorithmType::HillClimbing);
        int uniformColorDistribution = 0;

        // Initialize and run
        start_search(id, dim, max_iters, grid_type, algo_type, uniformColorDistribution);

        // Run until convergence or iteration limit
        GridResult result;
        for (int i = 0; i < max_iters; ++i) {
            result = step_search(id);
            // optional: stop early if score doesn’t improve
        }

        // Print result
        std::cout << "Best score: " << result.score << "\n";
        std::cout << "Iterations: " << result.iterations << "\n";
        std::cout << "Done.\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
