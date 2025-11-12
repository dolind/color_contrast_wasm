#pragma once
#include <vector>
#include "base_grid.hpp"


namespace color_contrast {
    void buildContrastTable();

    class GridImprove : public BaseGrid {
    public:
        GridImprove() = default;

        GridImprove(int dim, bool equalColorDistribution);

        GridImprove(const GridImprove &grid);

        ~GridImprove() override = default;

    private:
        // Improvement 4 Merge cells to grid to avoid redundancy
        std::vector<COilColor> m_colors;

        void evaluateGridScore() override;
    };
} // namespace color_contrast
