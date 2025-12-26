#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan_raii.hpp"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <stdexcept>
#include <iostream>
#include <utility>

namespace Vulkan {
	struct ContextInitInfo {
		int windowWidth = 0;
		int windowHeight = 0;
		const char* appName = nullptr;
		std::vector<const char*> validationLayers{};
	};

	class Context {
	private:
		Context();
		~Context();
		Context(Context const&) = delete;
		Context& operator=(Context const&) = delete;
		Context(Context&&) = delete;
		Context& operator=(Context&&) = delete;

		bool isInitialized;

		GLFWwindow* window;
		vk::raii::Context context;
		vk::raii::Instance instance;
		vk::raii::SurfaceKHR surface;
		vk::raii::PhysicalDevice physicalDevice;
		vk::raii::Device device;
		vk::raii::Queue graphicsQueue;

		const int WINDOW_WIDTH;
		const int WINDOW_HEIGHT;
		const char* windowName;

		const std::vector<const char*> VALIDATION_LAYERS;

		void initWindow(int const& WIDTH, int const& HEIGHT, const char* name);
		void initInstance(const std::vector<const char*>& VALIDATION);
		void initSurface();
		void initPhysicalDevice();
		void initDevice();

		std::pair<uint32_t, const char**> enumerateGlfwExtensions();
		bool verifyHaveGlfwExtensions(uint32_t const& needCount, const char**& needs);

	public:
		Context& self;

		[[nodiscard]] static Context& get();
		void init(ContextInitInfo const& initInfo);
	};
}
