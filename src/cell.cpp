#include "cell.hpp"
#include <cmath>      // std::abs

namespace color_contrast {
    static double rgb2lum(const RGB &rgbColor) {
        // identical to python: ((r*299)+(g*587)+(b*114)) / 1000
        return (rgbColor.r * 299 + rgbColor.g * 587 + rgbColor.b * 114) * 0.001;
    }

    Cell::Cell(const COilColor &color, int colorIndex)
        : m_rgbColor(color.rgbValue),
          m_name(color.name),
          m_luminance(rgb2lum(color.rgbValue)),m_colorIndex(colorIndex){
    }

    double Cell::lumcontrast(const double secondLuminance) const {
        return std::abs(m_luminance - secondLuminance);
    }

    double Cell::getContrastScore(const std::vector<double> &neighborPixels) const {
        double sumContrast = 0.0;

        for (double lum: neighborPixels)
            sumContrast += std::abs(m_luminance - lum);

        return sumContrast;
    }
}
