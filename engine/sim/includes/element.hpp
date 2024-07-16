#pragma once
#include <cstdint>
#include <random>
#include <stdint.h>
#include <elementType.hpp>

class Sim;

class Element {
public:
  void step(Sim &sim, std::uniform_int_distribution<std::mt19937::result_type> genBool);
  void swap(Sim &sim, uint32_t x, uint32_t y, Element &element);

public:
  uint32_t m_GridX;
  uint32_t m_GridY;
  ElementType m_Value;
  bool m_HasBeenDisplaced = false;
};
