#pragma once

#include <GLFW/glfw3.h>
#include <renderer.hpp>
#include <sim.hpp>

class Engine {
public:
  Engine();
  ~Engine();
  void Run();

private:
  void initWindow();

private:
  GLFWwindow *m_Window{nullptr};
  Renderer m_Renderer;
  tGrid m_WorldMatrix;
  Sim m_Sim;

  bool flag;
};
