#include "sim.hpp"

void updateCell(std::array<Cell, GRID_SIZE_X * GRID_SIZE_Y>& cell_state)
{
	for(uint32_t y = 0; y < GRID_SIZE_Y; ++y) {
		for(uint32_t x = 0; x < GRID_SIZE_X; ++x) {
			
			if (y == 0) {
				continue;
			}

			if (cell_state[y * GRID_SIZE_X + x].value == 1) {

				// cell below is empty
				if (cell_state[(y - 1) * GRID_SIZE_X + x].value == 0) {
					cell_state[(y - 1) * GRID_SIZE_X + x].value = 1;
					cell_state[y * GRID_SIZE_X + x].value = 0;
				}
				else if (x != 0 && x != GRID_SIZE_X - 1) {
					// cell below is filled
					if (cell_state[(y - 1) * GRID_SIZE_X + x - 1].value == 0) {
						cell_state[(y - 1) * GRID_SIZE_X + x - 1].value = 1;
						cell_state[y * GRID_SIZE_X + x].value = 0;
					}
					// cell to the left is filled
					else if (cell_state[(y - 1) * GRID_SIZE_X + x + 1].value == 0) {
						cell_state[(y - 1) * GRID_SIZE_X + x + 1].value = 1;
						cell_state[y * GRID_SIZE_X + x].value = 0;
					}

				}
			}

		}
	}
}