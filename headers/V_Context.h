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
		std::vector<const char*> deviceExtensions{};
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
		const std::vector<const char*> DEVICE_EXTENSIONS;
		vk::StructureChain<vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceVulkan11Features,
			vk::PhysicalDeviceVulkan13Features,
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> deviceFeatures;

		void initWindow(int const& WIDTH, int const& HEIGHT, const char* name);
		void initInstance(const std::vector<const char*>& VALIDATION);
		void initSurface();
		void initPhysicalDevice(std::vector<const char*> const& devExts);
		void initDevice();

		std::pair<uint32_t, const char**> enumerateGlfwExtensions();
		bool verifyHaveGlfwExtensions(uint32_t const& needCount, const char**& needs);

		std::vector<std::array<uint32_t, 4>> ratePhysicalDevices(std::vector<const char*> const& devExts);
		uint32_t judgePhysicalDevice(std::array<uint32_t, 4> rating);
		bool hasMinimumApiVersion(vk::raii::PhysicalDevice const& phyDev, int const& apiVersion);
		bool hasQueueFamily(vk::raii::PhysicalDevice const& phyDev, vk::QueueFlagBits const& familyBits);
		template <class... Ts>
		bool hasPhysicalDeviceFeatures(vk::raii::PhysicalDevice const& phyDev, vk::StructureChain<Ts...> const& features);
		template <class T>
		bool featureBundleSupported(T const& requested, T const& supported);
		bool hasPhysicalDeviceExtensions(vk::raii::PhysicalDevice const& phyDev, std::vector<const char*> const& extensions);

	public:
		Context& self;

		[[nodiscard]] static Context& get();
		void init(ContextInitInfo const& initInfo);
	};
}
