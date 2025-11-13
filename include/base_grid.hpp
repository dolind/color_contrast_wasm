#pragma once
#include <vector>
#include <unordered_map>
#include "cell.hpp"

namespace color_contrast {
    static constexpr std::int32_t COLORS_AVAILABLE = 25;


    struct GridResult {
        std::vector<COilColor> colors;
        double score{};
        int iterations{};
    };

    const std::unordered_map<int, COilColor> COLOR_MAP = {
        {1, {"White", {255, 255, 255}}},
        {2, {"Pale Blue", {90, 159, 232}}},
        {3, {"Azure Blue", {0, 96, 165}}},
        {4, {"Yellow Ocre", {219, 139, 45}}},
        {5, {"Mandarin", {255, 94, 31}}},
        {6, {"Intense Red", {197, 41, 49}}},
        {7, {"Ultramarine Blue", {0, 55, 168}}},
        {8, {"Ruby Red", {182, 36, 11}}},
        {9, {"Delft Blue", {0, 93, 197}}},
        {10, {"Geranium Lake Light", {221, 68, 94}}},
        {11, {"Celestial Blue", {0, 116, 151}}},
        {12, {"Red Brown", {148, 70, 55}}},
        {13, {"Celadon Green", {0, 151, 168}}},
        {14, {"Brown Madder", {93, 55, 51}}},
        {15, {"Cinnabar Green", {0, 161, 69}}},
        {16, {"Raw Umber", {89, 73, 59}}},
        {17, {"Green Medium", {0, 104, 61}}},
        {18, {"Burnt Umber", {85, 62, 56}}},
        {19, {"Pine Green", {37, 87, 75}}},
        {20, {"Reddish Brown Grey", {150, 133, 122}}},
        {21, {"Lemon Yellow", {255, 226, 37}}},
        {22, {"Grey Green", {125, 140, 120}}},
        {23, {"Yellow Deep", {255, 192, 0}}},
        {24, {"Black", {53, 53, 54}}}
    };


    class BaseGrid {
    public:
        BaseGrid() = default;

        explicit BaseGrid(const int dim)
            : m_width(dim), m_height(dim) {
        }
        virtual RGB getRGBAtIndex(int idx) const = 0;
        BaseGrid(const BaseGrid &grid) = default;

        virtual ~BaseGrid() = default;

        double getScore() const { return m_gridScore; }

        int width() const { return m_width; }
        int height() const { return m_height; }

        virtual Cell &at(const int x, const int y) { return m_Cells[y * m_width + x]; }
        virtual const Cell &at(const int x, const int y) const { return m_Cells[y * m_width + x]; }

        void printToConsole() const;

        virtual GridResult toResult() const;

        void setIterations(const int i) { iterationCount = i; }
        int iterations() const { return iterationCount; }

    protected:
        virtual void evaluateGridScore() = 0; // like Python, computes score
        int iterationCount = 0;
        std::unordered_map<int, COilColor> colors = COLOR_MAP;

        std::vector<Cell> m_Cells;
        std::vector<double> m_Luminance;
        double m_gridScore = 0.0;

        int m_width{};
        int m_height{};
    };
} // namespace color_contrast
