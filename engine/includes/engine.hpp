#pragma once

#include <renderer.hpp>

namespace Engine {
    void f(std::array<Cell, GRID_SIZE_X * GRID_SIZE_Y>& cell_state);
    void run();
}