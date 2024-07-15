#pragma once
#include <cstdint>
#include <stdint.h>
#include <elementType.hpp>

class Sim;

class Element {
public:
  void step(Sim &sim);
  void swap(Sim &sim, uint32_t x, uint32_t y, Element &element);

public:
  uint32_t m_GridX;
  uint32_t m_GridY;
  ElementType m_Value;
};
