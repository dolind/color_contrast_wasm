#include "improved_grid.hpp"
#include <algorithm>
#include <random>
#include <numeric>  // std::iota
#include <cmath>
#include <iostream>
#include <iomanip>
#include <unordered_set>

#include "contrast_table.hpp"

namespace color_contrast {
    //-----------------------------------------------------------
    // Helper to generate unique shuffled indices (original logic)
    //-----------------------------------------------------------
    //Improvement 2: static rng
    static thread_local std::mt19937 eng{std::random_device{}()};
    static thread_local std::uniform_int_distribution dist(1, COLORS_AVAILABLE);

    //Improvement 3: assignment of colors
    std::vector<std::int32_t> getUniqueColorNum2(std::uint32_t colorCount, bool equalColorDistribution) {
        std::vector<std::int32_t> pool(COLORS_AVAILABLE);
        std::iota(pool.begin(), pool.end(), 1);
        std::shuffle(pool.begin(), pool.end(), eng);

        if (colorCount <= COLORS_AVAILABLE) {
            pool.resize(colorCount);
            return pool;
        }

        std::vector result(pool.begin(), pool.end());
        result.reserve(colorCount);

        // then fill remaining slots with duplicates (sampling with replacement)
        if (equalColorDistribution) {
            // round-robin advance
            // use same palette again
            std::size_t nextIndex = 0;
            while (result.size() < colorCount) {
                result.push_back(pool[nextIndex]);
                nextIndex = (nextIndex + 1) % pool.size();
            }
        } else {
            // then fill remaining slots with duplicates (sampling with replacement)
            while (result.size() < colorCount) {
                result.push_back(dist(eng)); // repeats allowed ✔
            }
        }
        return result;
    }

    int GridImprove::countDistinctColors() const {
        std::unordered_set<int> s(m_ColorIndices.begin(), m_ColorIndices.end());
        return (int)s.size();
    }

    GridImprove::GridImprove(const GridImprove &grid)
        : BaseGrid(grid) {
    }

    constexpr auto rgb2lum = [](const RGB &rgb) {
        return 0.299 * rgb.r + 0.587 * rgb.g + 0.114 * rgb.b;
    };

    GridImprove::GridImprove(int dim, bool equalColorDistribution)
        : BaseGrid(dim) {
        const auto colorNumbers = getUniqueColorNum2(dim * dim, equalColorDistribution);

        m_colors.reserve(dim * dim);
        m_Luminance.reserve(dim * dim);

        for (std::uint32_t colorNum: colorNumbers) {
            COilColor newColor = colors[colorNum];
            m_colors.emplace_back(newColor);
            m_Luminance.push_back(rgb2lum(newColor.rgbValue));
            m_ColorIndices.push_back(colorNum - 1);
        }

        GridImprove::evaluateGridScore();
    }

    // Improvement 1: avoid std::vector allocation and sum for each cell
    // Inline cell logic for contrast score
    // Improvement 4 Merge cells to grid to avoid redundancy
    // Improvement 5 Lookup table
    void GridImprove::evaluateGridScore()
    {
        const int w = m_width;
        const int h = m_height;
        const int total = w * h;

        double score = 0.0;

        // Offsets for 3×3 neighborhood
        static const int dx[9] = { -1, 0, 1, -1, 0, 1, -1, 0, 1 };
        static const int dy[9] = { -1,-1,-1,  0, 0, 0,  1, 1, 1 };

        for (int idx = 0; idx < total; ++idx)
        {
            const int x = idx % w;
            const int y = idx / w;
            const int centerIdx = m_ColorIndices[idx];

            double cellScore = 0.0;

            // Scan 3×3 neighborhood (with black border padding)
            for (int n = 0; n < 9; ++n)
            {
                const int nx = x + dx[n];
                const int ny = y + dy[n];

                int neighborIdx = 0; // default = black border (index 0)

                if (nx >= 0 && nx < w && ny >= 0 && ny < h)
                    neighborIdx = m_ColorIndices[ny * w + nx];

                cellScore += contrastTable[centerIdx][neighborIdx];
            }

            score += cellScore;
        }

        m_gridScore = score;
    }
    GridResult GridImprove::toResult() const
    {
        GridResult result;
        result.score = m_gridScore;
        result.iterations = iterationCount;
        result.colors.reserve(m_width * m_height);

        for (int y = 0; y < m_height; ++y)
        {
            for (int x = 0; x < m_width; ++x)
            {
                const COilColor& color = m_colors[y * m_width + x];
                result.colors.push_back({ color.name, color.rgbValue });
            }
        }

        return result;
    }

    void GridImprove::swapCells(int a, int b) {
        std::swap(m_ColorIndices[a], m_ColorIndices[b]);
        std::swap(m_colors[a], m_colors[b]);
        std::swap(m_Luminance[a], m_Luminance[b]);
    }

    void GridImprove::recolorCell(int idx, int newColorIndex) {
        m_ColorIndices[idx] = newColorIndex;
        const COilColor& c = colors[newColorIndex + 1];
        m_colors[idx] = c;
        m_Luminance[idx] = 0.299*c.rgbValue.r + 0.587*c.rgbValue.g + 0.114*c.rgbValue.b;
    }
    void GridImprove::updateDelta(int idx) { evaluateGridScore(); }
    void GridImprove::updateDelta(int a, int b) { evaluateGridScore(); }

    void GridImprove::restoreDelta(int idx) { evaluateGridScore(); }
    void GridImprove::restoreDelta(int a, int b) { evaluateGridScore(); }

} // namespace color_contrast
