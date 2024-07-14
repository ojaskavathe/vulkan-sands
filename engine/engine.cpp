#include "includes/utils.hpp"
#include <engine.hpp>
#include <sim.hpp>
#include <thread>

Engine::Engine() : m_Sim(m_WorldMatrix) {
  initWindow();
  m_Renderer.init(m_Window, &m_WorldMatrix);
}

Engine::~Engine() {
  glfwDestroyWindow(m_Window);
  glfwTerminate();
}

void Engine::Run() {
  m_Renderer.render();
  while (!glfwWindowShouldClose(m_Window)) {
    auto now = std::chrono::high_resolution_clock::now();
    auto end = now + std::chrono::milliseconds(MIN_FRAME_TIME);

    glfwPollEvents();
    m_Sim.updateCell();
    m_Renderer.render();

    std::this_thread::sleep_until(end);
  }
}

void Engine::initWindow() {
  glfwInit();
  glfwSetErrorCallback(glfwErrorCallback);

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  m_Window = glfwCreateWindow(WIDTH, HEIGHT, "HAHAHA", nullptr, nullptr);

  glfwSetWindowUserPointer(m_Window, this);

  glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow *win, int x, int y) {
    Renderer *app = reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(win));
    app->framebufferResized = true;
  });
}
