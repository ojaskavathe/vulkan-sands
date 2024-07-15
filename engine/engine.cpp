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
  // X errors on close if i don't render before the main loop, no idea why
  m_Renderer.render();

  auto last_time = std::chrono::high_resolution_clock::now();

  while (!glfwWindowShouldClose(m_Window)) {
    auto now = std::chrono::high_resolution_clock::now();
    auto deltaTime = now - last_time;
    auto end = now + std::chrono::milliseconds(MIN_FRAME_TIME);

    glfwPollEvents();
    m_Sim.step();
    m_Renderer.render();

    if (glfwGetWindowAttrib(m_Window, GLFW_HOVERED)) {
      double xpos, ypos;
      glfwGetCursorPos(m_Window, &xpos, &ypos);
      if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        m_Sim.mouse(xpos, ypos);
      } else if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_RIGHT) ==
                 GLFW_PRESS) {
        m_Sim.mouse(xpos, ypos, true);
      }
    }

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
