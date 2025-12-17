#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Vulkan {
	class Instance {
	private:
		vk::raii::Context context;
		vk::raii::Instance instance;

		bool hasGlfwExtensions();
	public:
		Instance();
	};
}