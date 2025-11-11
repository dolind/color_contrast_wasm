#pragma once
#include <vector>
#include <unordered_map>
#include <cstdint>
#include "cell.hpp"

namespace color_contrast {

static constexpr std::int32_t COLORS_AVAILABLE = 25;



struct GridResult {
    std::vector<COilColor> colors;
    double score;
    int iterations;
};


class Grid {

public:
    Grid()=default;
    Grid(int width, int height);
    Grid(const Grid& grid);
    ~Grid() = default;

    double getScore() const { return m_gridScore; }

    int width() const { return m_width; }
    int height() const { return m_height; }

    Cell& at(int x, int y)             { return m_Cells[y * m_width + x]; }
    const Cell& at(int x, int y) const { return m_Cells[y * m_width + x]; }
    void printToConsole() const;
    GridResult toResult() const;
    void setIterations(int i) { iterationCount = i; }
    int iterations() const { return iterationCount; }
private:
    void evaluateGridScore();         // like Python, computes score
    int iterationCount = 0;
    std::unordered_map<int, COilColor> colors = {
        {1, {"rose madder",          {227,  38,  54}}},
        {2, {"dunkelgrün",           {  0, 136,   0}}},
        {3, {"blue cobalt",          {  0,  71, 171}}},
        {4, {"rose pink",            {255, 102, 204}}},
        {5, {"red",                  {255,   0,   0}}},
        {6, {"sky blue",             {135, 206, 235}}},
        {7, {"ochre --gray",         {204, 119,  34}}},
        {8, {"cinnabar",             {227,  66,  52}}},
        {9, {"cobalt green",         { 61, 145,  64}}},
        {10,{"ochre",                {204, 119,  34}}},
        {11,{"violet",               {238, 130, 238}}},
        {12,{"dunkelgrün",           {  0, 136,   0}}},
        {13,{"olivgrün",             { 66,  70,  50}}},
        {14,{"pale orange",          {255, 224, 194}}},
        {15,{"yellow green",         {154, 205,  50}}},
        {16,{"cadmium green",        {  0, 107,  60}}},
        {17,{"chrome yellow",        {255, 167,   0}}},
        {18,{"deep madder",          {161,  21,  50}}},
        {19,{"fresh rose",           {168,  97,  98}}},
        {20,{"fresh rose --white",   {168,  97,  98}}},
        {21,{"van dyke brown",       { 88,  70,  48}}},
        {22,{"orange",               {255, 204,   0}}},
        {23,{"ultramarin blue",      { 65, 102, 245}}},
        {24,{"dark brown",           {101,  67,  33}}},
        {25,{"deep madder",          {161,  21,  50}}}
    };

    std::vector<Cell> m_Cells;
    std::vector<double> m_Luminance;
    double m_gridScore = 0.0;

    int m_width;
    int m_height;
};

} // namespace color_contrast
