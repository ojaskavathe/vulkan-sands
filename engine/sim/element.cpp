#include <element.hpp>
#include <sim.hpp>

void Element::step(
    Sim &sim,
    std::uniform_int_distribution<std::mt19937::result_type> genBool) {

  uint32_t curr_X = m_GridX;
  uint32_t curr_Y = m_GridY;
  Element self = *this;

  switch (m_Value) {
  case ElementType::Sand: {

    std::pair<int, int> positionsToTry[3] = {
        {0, -1},
        {-1, -1},
        {1, -1},
    };

    for (auto pos : positionsToTry) {
      auto target = sim.get(m_GridX + pos.first, m_GridY + pos.second);
      if (target && !target.value().m_HasBeenDisplaced &&
          target.value().m_Value != ElementType::Sand) {
        swap(sim, curr_X + pos.first, curr_Y + pos.second, target.value());
        break;
      }
    }
    break;
  }
  case ElementType::Water: {
    std::pair<int, int> positionsToTry[5] = {
        {0, -1},
        {-1, -1},
        {1, -1},
        {-1, 0},
        {1, 0},
    };

    for (int i = 0; i < 5; i++) {
      auto pos = positionsToTry[i];
      auto target = sim.get(m_GridX + pos.first, m_GridY + pos.second);
      if (target &&
          target.value().m_Value == ElementType::Air) {
        swap(sim, curr_X + pos.first, curr_Y + pos.second, target.value());
        if (i == 4) {
          auto temp = positionsToTry[4];
          positionsToTry[4] = positionsToTry[3];
          positionsToTry[3] = temp;
        }
        break;
      }
    }
    break;
  }
  case ElementType::Air:
    break;
  }
}

void Element::swap(Sim &sim, uint32_t x, uint32_t y, Element &element) {
  Element self = *this;
  sim.set(m_GridX, m_GridY, element);
  sim.set(x, y, self);
};
