#pragma once
#include "base_grid.hpp"

struct StepInfo {
    double score;
    int iterations;
};


void start_search(int id, int dim, int max_iters, int grid_type, int algo_type, int uniformColorDistribution);

StepInfo step_search_info(int id);

// We use this function to avoid returning a struct which contains the grid, but used shared memory
void export_grid_rgb(int id, std::uint8_t* out, int max_len);

int get_color_index(int id, int cell) ;