#include "V_Window.h"
#include <stdexcept>
#include <iostream>

namespace Vulkan {
	Window::Window(int const& width, int const& height, const char* const& name) : WIDTH{width}, HEIGHT{height} {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(WIDTH, HEIGHT, name, nullptr, nullptr);

		if (window == NULL) throw std::runtime_error("GLFW window creation failed");
		else std::cout << "Window created\n";
	}

	Window::~Window() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}
}