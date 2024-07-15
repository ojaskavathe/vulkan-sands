#include <config.hpp>
#include <cstdint>
#include <sim.hpp>

Sim::Sim(tGrid &worldMatrix) : m_WorldMatrix(worldMatrix) {
  // initial state

  for (uint32_t y = 0; y < GRID_SIZE_Y; ++y) {
    for (uint32_t x = 0; x < GRID_SIZE_X; ++x) {
      m_ElementsMatrix[y * GRID_SIZE_X + x] = {x, y, ElementType::Air};
    }
  }

  for (int y = 2; y < GRID_SIZE_Y/2; ++y) {
    m_ElementsMatrix[y * GRID_SIZE_X + GRID_SIZE_X / 2 - 4].m_Value = ElementType::Sand;
    m_ElementsMatrix[y * GRID_SIZE_X + GRID_SIZE_X / 2].m_Value = ElementType::Sand;
    m_ElementsMatrix[y * GRID_SIZE_X + GRID_SIZE_X / 2 + 4].m_Value = ElementType::Sand;
  }
}

void Sim::set(uint32_t x, uint32_t y, Element &element) {
  m_ElementsMatrix[y * GRID_SIZE_X + x] = element;
  m_ElementsMatrix[y * GRID_SIZE_X + x].m_GridX = x;
  m_ElementsMatrix[y * GRID_SIZE_X + x].m_GridY = y;

  m_WorldMatrix[y * GRID_SIZE_X + x].value = static_cast<uint32_t>(element.m_Value);
}

std::optional<Element> Sim::get(uint32_t x, uint32_t y) {
  if (insideBounds(x, y)) {
    return m_ElementsMatrix[y * GRID_SIZE_X + x];
  }
  return std::nullopt;
}

bool Sim::insideBounds(uint32_t x, uint32_t y) {
  return x >= 0 && y >= 0 && x < GRID_SIZE_X && y < GRID_SIZE_Y;
}

void Sim::updateCell() {
  for (Element &element : m_ElementsMatrix) {
    if (element.m_Value != ElementType::Air)
      element.step(*this);
  }
  // for (uint32_t y = 0; y < GRID_SIZE_Y; ++y) {
  //   for (uint32_t x = 0; x < GRID_SIZE_X; ++x) {
  //     m_WorldMatrix[y * GRID_SIZE_X + x].value = m_ElementsMatrix[y * GRID_SIZE_X + x].m_Value;
  //   }
  // }
}
