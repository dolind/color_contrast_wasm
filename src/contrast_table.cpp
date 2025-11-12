#include "contrast_table.hpp"
#include "base_grid.hpp"
#include <cmath>
#include <iostream>

namespace color_contrast {

    std::vector<std::vector<double>> contrastTable;

    static double rgb2lum(const RGB& rgbColor)
    {
        return (rgbColor.r * 299 + rgbColor.g * 587 + rgbColor.b * 114) * 0.001;
    }

    // Improvement 5: precompute contrast and use in improvement 6
    void buildContrastTable()
    {
        if (COLOR_MAP.contains(0))
        {
            std::cerr << "⚠️  Warning: COLOR_MAP already contains an entry with ID 0.\n"
                         "   Cannot insert black as background; skipping extra black row.\n";
        }
        const int COLORS_AVAILABLE = static_cast<int>(COLOR_MAP.size());
        contrastTable.assign(COLORS_AVAILABLE + 1, std::vector<double>(COLORS_AVAILABLE + 1, 0.0));

        // insert all colors starting at 1
        for (const auto& [aID, colorA] : COLOR_MAP)
        {
            double lumA = rgb2lum(colorA.rgbValue);

            for (const auto& [bID, colorB] : COLOR_MAP)
            {
                double lumB = rgb2lum(colorB.rgbValue);
                contrastTable[aID][bID] = std::abs(lumA - lumB);
            }
        }

        // insert black as color 0
        const RGB black = {0, 0, 0};
        const double lumBlack = rgb2lum(black);

        for (const auto& [id, color] : COLOR_MAP)
        {
            double lum = rgb2lum(color.rgbValue);
            double diff = std::abs(lum - lumBlack);

            contrastTable[0][id] = diff; // black → color
            contrastTable[id][0] = diff; // color → black (symmetric)
        }

        std::cout << "Contrast table built with " << COLORS_AVAILABLE << " colors.\n";
    }

} // namespace color_contrast
