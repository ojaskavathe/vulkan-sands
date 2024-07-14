#include <sim.hpp>

Sim::Sim(tGrid& worldMatrix): m_WorldMatrix(worldMatrix) {
  // initial state
  for (int y = 0; y < GRID_SIZE_Y; ++y) {
    m_WorldMatrix[y * GRID_SIZE_X + GRID_SIZE_X / 2 - 4].value = 1;
    m_WorldMatrix[y * GRID_SIZE_X + GRID_SIZE_X / 2].value = 1;
    m_WorldMatrix[y * GRID_SIZE_X + GRID_SIZE_X / 2 + 4].value = 1;
  }
}

void Sim::updateCell() {
  for (uint32_t y = 0; y < GRID_SIZE_Y; ++y) {
    for (uint32_t x = 0; x < GRID_SIZE_X; ++x) {

      if (y == 0) {
        continue;
      }

      if (m_WorldMatrix[y * GRID_SIZE_X + x].value == 1) {

        // cell below is empty
        if (m_WorldMatrix[(y - 1) * GRID_SIZE_X + x].value == 0) {
          m_WorldMatrix[(y - 1) * GRID_SIZE_X + x].value = 1;
          m_WorldMatrix[y * GRID_SIZE_X + x].value = 0;
        } else if (x != 0 && x != GRID_SIZE_X - 1) {
          // cell below is filled
          if (m_WorldMatrix[(y - 1) * GRID_SIZE_X + x - 1].value == 0) {
            m_WorldMatrix[(y - 1) * GRID_SIZE_X + x - 1].value = 1;
            m_WorldMatrix[y * GRID_SIZE_X + x].value = 0;
          }
          // cell to the left is filled
          else if (m_WorldMatrix[(y - 1) * GRID_SIZE_X + x + 1].value == 0) {
            m_WorldMatrix[(y - 1) * GRID_SIZE_X + x + 1].value = 1;
            m_WorldMatrix[y * GRID_SIZE_X + x].value = 0;
          }
        }
      }
    }
  }
}
