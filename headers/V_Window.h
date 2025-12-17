#pragma once

#include <GLFW/glfw3.h>

namespace Vulkan {
	class Window {
	private:
		GLFWwindow* window;
		const int WIDTH;
		const int HEIGHT;

	public:
		Window(int const& width, int const& height, const char* const& name);
		~Window();
	};
}