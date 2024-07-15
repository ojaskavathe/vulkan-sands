#pragma once

#include <cell.hpp>
#include <array>

const uint32_t GRID_SIZE_X = 64;
const uint32_t GRID_SIZE_Y = 64;
const uint32_t MIN_FRAME_TIME = 32;

typedef std::array<Cell, GRID_SIZE_X * GRID_SIZE_Y> tGrid;

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 800;
const uint32_t MAX_FRAMES_IN_FLIGHT = 2;
