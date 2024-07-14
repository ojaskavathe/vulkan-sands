#include <iostream>
inline void glfwErrorCallback(int code, const char* description)
{
	std::cerr << "[GLFW]: " << code << ": " << description;
	std::cerr << std::endl;
}
