#pragma once

#include <config.hpp>

class Sim {
public:
  Sim(tGrid& m_WorldMatrix);
  void updateCell();

private:
  tGrid& m_WorldMatrix;
};
