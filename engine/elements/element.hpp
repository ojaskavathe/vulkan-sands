#pragma once
#include <stdint.h>

struct Element {
  alignas(16) uint32_t value = 0;
};
