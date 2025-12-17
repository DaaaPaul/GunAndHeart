#pragma once

#include "V_Window.h"
#include "V_Instance.h"

namespace Vulkan {
	class Engine {
	private:
		Window window;
		Instance instance;

	public:
		Engine(int const& width, int const& height);
	};
}