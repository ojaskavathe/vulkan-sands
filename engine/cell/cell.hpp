#pragma once
#include <stdint.h>

struct Cell {
  alignas(16) uint32_t value = 0;
};
