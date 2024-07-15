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

  for (int y = 2; y < GRID_SIZE_Y / 2; ++y) {
    m_ElementsMatrix[y * GRID_SIZE_X + GRID_SIZE_X / 2 - 4].m_Value =
        ElementType::Sand;
    m_ElementsMatrix[y * GRID_SIZE_X + GRID_SIZE_X / 2].m_Value =
        ElementType::Sand;
    m_ElementsMatrix[y * GRID_SIZE_X + GRID_SIZE_X / 2 + 4].m_Value =
        ElementType::Sand;
  }
}

void Sim::set(uint32_t x, uint32_t y, Element &element) {
  if (insideBounds(x, y)) {
    m_ElementsMatrix[y * GRID_SIZE_X + x] = element;
    m_ElementsMatrix[y * GRID_SIZE_X + x].m_GridX = x;
    m_ElementsMatrix[y * GRID_SIZE_X + x].m_GridY = y;

    m_WorldMatrix[y * GRID_SIZE_X + x].value =
        static_cast<uint32_t>(element.m_Value);
  }
}

void Sim::set(uint32_t x, uint32_t y, ElementType type) {
  if (insideBounds(x, y)) {
    m_ElementsMatrix[y * GRID_SIZE_X + x].m_Value = type;

    m_WorldMatrix[y * GRID_SIZE_X + x].value =
        static_cast<uint32_t>(type);
  }
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

void Sim::mouse(double xpos, double ypos, bool sink) {
  uint32_t x = (xpos / WIDTH) * GRID_SIZE_X;
  uint32_t y = GRID_SIZE_Y - (ypos / HEIGHT) * GRID_SIZE_Y;

  for (int i = -3; i < 4; i++) {
    for (int j = -3; j < 4; j++) {
      set(x+i, y+j, sink ? ElementType::Air : ElementType::Sand);
    }
  }
}

void Sim::step() {
  for (Element &element : m_ElementsMatrix) {
    if (element.m_Value != ElementType::Air)
      element.step(*this);
  }
}
