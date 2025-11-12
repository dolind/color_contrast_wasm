#pragma once
#include <vector>
#include <unordered_map>
#include <cstdint>
#include "base_grid.hpp"
#include "cell.hpp"


namespace color_contrast {
    class Grid : public BaseGrid {
    public:
        Grid() = default;

        Grid(int dim, bool equalColorDistribution);

        Grid(const Grid &grid);

        ~Grid() = default;

    private:
        void evaluateGridScore() override;
    };
} // namespace color_contrast
