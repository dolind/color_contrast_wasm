#include "grid.hpp"
#include <algorithm>
#include <random>
#include <numeric>  // std::iota
#include <cmath>
#include <iostream>
#include <iomanip>

namespace color_contrast {
    //-----------------------------------------------------------
    // Helper to generate unique shuffled indices (original logic)
    //-----------------------------------------------------------
    std::vector<std::int32_t> getUniqueColorNum(const std::uint32_t colorCount, const bool equalColorDistribution) {
        std::mt19937 eng{std::random_device{}()};
        std::uniform_int_distribution dist(1, COLORS_AVAILABLE);

        std::vector<std::int32_t> result;
        result.reserve(colorCount);

        if (colorCount <= COLORS_AVAILABLE) {
            std::vector<std::int32_t> pool(COLORS_AVAILABLE);
            std::iota(pool.begin(), pool.end(), 1);
            std::shuffle(pool.begin(), pool.end(), eng);
            pool.resize(colorCount);
            return pool;
        }
        std::vector<std::int32_t> pool(COLORS_AVAILABLE);
        std::iota(pool.begin(), pool.end(), 1);
        std::shuffle(pool.begin(), pool.end(), eng);

        // use each color once
        result.insert(result.end(), pool.begin(), pool.end());

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

    Grid::Grid(const int dim, const bool equalColorDistribution)
        : BaseGrid(dim) {
        const auto colorNumbers = getUniqueColorNum(dim * dim, equalColorDistribution);

        m_Cells.reserve(dim * dim);
        m_Luminance.reserve(dim * dim);

        for (std::uint32_t colorNum: colorNumbers) {
            COilColor newColor = colors[colorNum];
            m_Cells.emplace_back(newColor, colorNum);
            m_Luminance.push_back(m_Cells.back().getLuminance());
        }

        Grid::evaluateGridScore();
    }

    //-----------------------------------------------------------
    // Scoring logic — same as original Python + OpenCV behavior
    //-----------------------------------------------------------
    void Grid::evaluateGridScore() {
        m_gridScore = 0.0;

        for (int x = 0; x < m_width; ++x) {
            for (int y = 0; y < m_height; ++y) {
                int idx = y * m_width + x;
                // gather neighborhood luminance (3×3, no padding)
                std::vector<double> neighbours;
                neighbours.reserve(9);

                for (int dx = -1; dx <= 1; ++dx) {
                    for (int dy = -1; dy <= 1; ++dy) {
                        int nx = x + dx;
                        int ny = y + dy;

                        // check boundary (replaces padded cv::Mat copyTo hack)
                        double neighborLum = 0.0; // default: black border

                        if (nx >= 0 && nx < m_width &&
                            ny >= 0 && ny < m_height) {
                            neighborLum = m_Luminance[ny * m_width + nx];
                        }

                        neighbours.push_back(neighborLum);
                    }
                }

                // ask cell to score this neighborhood
                double cellScore = m_Cells[idx].getContrastScore(neighbours);
                m_gridScore += cellScore;
            }
        }
    }

    void BaseGrid::printToConsole() const {
        std::cout << "Grid (" << m_width << "x" << m_height << ") score=" << m_gridScore << "\n";
        std::cout << "------------------------------------------------------------\n";

        for (int y = 0; y < m_height; ++y) {
            for (int x = 0; x < m_width; ++x) {
                const auto &cell = at(x, y);

                // print truncated / padded color name: 12 chars wide
                std::cout << std::left << std::setw(18) << cell.getName();
            }
            std::cout << "\n";
        }

        std::cout << "------------------------------------------------------------\n\n";
    }


    GridResult BaseGrid::toResult() const {
        GridResult result;
        result.score = m_gridScore;
        result.iterations = iterationCount;
        result.colors.reserve(m_width * m_height);

        for (int y = 0; y < m_height; ++y) {
            for (int x = 0; x < m_width; ++x) {
                const auto &cell = at(x, y);
                result.colors.push_back({cell.getName(), cell.getRGBColor()});
            }
        }

        return result;
    }
} // namespace color_contrast
