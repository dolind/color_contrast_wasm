#pragma once
#include "base_grid.hpp"



namespace color_contrast {
    class Grid : public BaseGrid {
    public:
        Grid() = default;

        Grid(int dim, bool equalColorDistribution);

        Grid(const Grid &grid)= default;

        ~Grid() override = default;

        RGB getRGBAtIndex(int idx) const override {
            return m_Cells[idx].getRGBColor();
        }
    private:
        void evaluateGridScore() override;
    };
} // namespace color_contrast
