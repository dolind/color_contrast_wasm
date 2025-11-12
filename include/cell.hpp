#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace color_contrast {
    struct RGB {
        std::uint8_t r;
        std::uint8_t g;
        std::uint8_t b;
    };

    struct COilColor {
        std::string name;
        RGB rgbValue;
    };



    class Cell {
    public:
        explicit Cell(const COilColor &color);

        ~Cell() = default;


        double getLuminance() const { return m_luminance; }
        RGB getRGBColor() const { return m_rgbColor; }
        std::string getName() const { return m_name; }


        double lumcontrast(double secondLuminance) const;


        double getContrastScore(const std::vector<double> &neighborPixels) const;

    private:
        RGB m_rgbColor;
        std::string m_name;
        double m_luminance;
    };
}
