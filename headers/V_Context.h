#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan_raii.hpp"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <stdexcept>
#include <iostream>

namespace Vulkan {
	class Context {
	private:
		Context();
		~Context();
		Context(Context const&) = delete;
		Context& operator=(Context const&) = delete;

		GLFWwindow* window;
		vk::raii::Context context;
		vk::raii::Instance instance;
		vk::raii::SurfaceKHR surface;
		vk::raii::PhysicalDevice physicalDevice;
		vk::raii::Device device;
		vk::raii::Queue graphicsQueue;

		void initWindow();
		void initInstance();
		void initSurface();
		void initPhysicalDevice();
		void initDevice();

	public:
		[[nodiscard]] static Context& get();
		void init();
	};
}
