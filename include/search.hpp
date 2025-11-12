#pragma once
#include "base_grid.hpp"

void start_search(int id, int dim, int max_iters, int grid_type, int algo_type, int uniformColorDistribution);

color_contrast::GridResult step_search(int id) ;