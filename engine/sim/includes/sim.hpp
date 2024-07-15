#pragma once

#include <config.hpp>
#include <element.hpp>

#include <array>
#include <cstdint>
#include <optional>

class Sim {
public:
  Sim(tGrid &worldMatrix);
  void step();

  void set(uint32_t x, uint32_t y, Element &elem);
  void set(uint32_t x, uint32_t y, ElementType type);

  bool insideBounds(uint32_t x, uint32_t y);
  std::optional<Element> get(uint32_t x, uint32_t y);

  void mouse(double xpos, double ypos, bool sink = false);

private:
  tGrid &m_WorldMatrix;
  std::array<Element, GRID_SIZE_Y * GRID_SIZE_X> m_ElementsMatrix;
};
