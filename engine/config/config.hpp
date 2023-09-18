#pragma once
#include <stdint.h>
#include <array>

const uint32_t GRID_SIZE_X = 128;
const uint32_t GRID_SIZE_Y = 128;
const uint32_t MIN_FRAME_TIME = 16;

struct Cell {
	alignas(16) uint32_t value;
};

typedef std::array<Cell, GRID_SIZE_X * GRID_SIZE_Y> grid;

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 800;
const uint32_t MAX_FRAMES_IN_FLIGHT = 2;