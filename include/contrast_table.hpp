#pragma once
#include <vector>

namespace color_contrast {

    extern std::vector<std::vector<double>> contrastTable;

    void buildContrastTable();

}