#pragma once

#include <element.hpp>
#include <array>
#include <stdint.h>

const uint32_t GRID_SIZE_X = 128;
const uint32_t GRID_SIZE_Y = 128;
const uint32_t MIN_FRAME_TIME = 64;

typedef std::array<Element, GRID_SIZE_X * GRID_SIZE_Y> tGrid;

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 800;
const uint32_t MAX_FRAMES_IN_FLIGHT = 2;
