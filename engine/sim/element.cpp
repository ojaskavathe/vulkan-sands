#include "element.hpp"
#include "sim.hpp"
#include <vulkan/vulkan_raii.hpp>

void Element::step(Sim &sim) {
  auto below = sim.get(m_GridX, m_GridY - 1);

  if (!below)
    return;

  uint32_t curr_X = m_GridX;
  uint32_t curr_Y = m_GridY;
  Element self = *this;

  Element belowDirect = below.value();
  auto belowLeft = sim.get(m_GridX - 1, m_GridY - 1);
  auto belowRight = sim.get(m_GridX + 1, m_GridY - 1);
  if (belowDirect.m_Value == ElementType::Air) {
    swap(sim, curr_X, curr_Y - 1, belowDirect);
  } else if (belowLeft && belowLeft.value().m_Value == ElementType::Air) {
    swap(sim, curr_X - 1, curr_Y - 1, belowLeft.value());
  } else if (belowRight && belowRight.value().m_Value == ElementType::Air) {
    swap(sim, curr_X + 1, curr_Y - 1, belowRight.value());
  }
}

void Element::swap(Sim &sim, uint32_t x, uint32_t y, Element &element){
  Element self = *this;
  sim.set(m_GridX, m_GridY, element);
  sim.set(x, y, self);
};
