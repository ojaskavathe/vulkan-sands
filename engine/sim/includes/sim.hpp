#pragma once

#include <element.hpp>
#include <config.hpp>

#include <array>
#include <cstdint>
#include <optional>

class Sim {
public:
  Sim(tGrid &worldMatrix);
  void updateCell();

  void set(uint32_t x, uint32_t y, Element &elem);
  bool insideBounds(uint32_t x, uint32_t y);
  std::optional<Element> get(uint32_t x, uint32_t y);

private:
  tGrid &m_WorldMatrix;
  std::array<Element, GRID_SIZE_Y * GRID_SIZE_X> m_ElementsMatrix;
};
