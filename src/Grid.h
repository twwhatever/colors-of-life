#pragma once

#include <vector>

enum class CellType {
    Empty,
    Food,
    Teammate,
    Opponent,
};

using Grid = std::vector<std::vector<CellType>>;