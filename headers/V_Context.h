#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan_raii.hpp"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <stdexcept>
#include <iostream>
#include <utility>
#include <tuple>

#define doubleApiVersionToVkApiVersion(d) \
	((d) == 1.1 ? vk::ApiVersion11 : \
     (d) == 1.2 ? vk::ApiVersion12 : \
     (d) == 1.3 ? vk::ApiVersion13 : \
     (d) == 1.4 ? vk::ApiVersion14 : vk::ApiVersion10)

namespace Vulkan {
	template <class... Ts>
	struct ContextInitInfo {
		int windowWidth = 0;
		int windowHeight = 0;
		const char* appName = nullptr;
		double apiVersion = 0.0;
		std::vector<const char*> validationLayers{};
		std::vector<const char*> deviceExtensions{};
		vk::StructureChain<Ts...> deviceFeatures{};
		std::vector<std::tuple<vk::QueueFlagBits, uint32_t, float>> queueFamilies{};
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
		std::vector<std::vector<vk::raii::Queue>> queues;

		void initWindow(int const& WIDTH, int const& HEIGHT, const char* name);
		void initInstance(double const& apiVersion, const std::vector<const char*>& validLays);
		void initSurface();
		template <class... Ts>
		void initPhysicalDevice(double const& apiVersion, std::vector<const char*> const& devExts, vk::StructureChain<Ts...> const& devFeats, std::vector<std::tuple<vk::QueueFlagBits, uint32_t, float>> const& queuesInfo);
		template <class... Ts>
		void initDevice(std::vector<const char*> const& devExts, vk::StructureChain<Ts...> const& devFeats, std::vector<std::tuple<vk::QueueFlagBits, uint32_t, float>> const& queuesInfo);

		std::pair<uint32_t, const char**> enumerateGlfwExtensions();
		bool verifyHaveGlfwExtensions(uint32_t const& needCount, const char**& needs);

		template <class... Ts>
		std::vector<std::array<uint32_t, 4>> ratePhysicalDevices(double const& apiVersion, std::vector<const char*> const& devExts, vk::StructureChain<Ts...> const& devFeats, std::vector<std::tuple<vk::QueueFlagBits, uint32_t, float>> const& queuesInfo);
		uint32_t judgePhysicalDevice(std::array<uint32_t, 4> rating);
		bool hasMinimumApiVersion(vk::raii::PhysicalDevice const& phyDev, uint32_t const& apiVersion);
		bool hasQueueFamily(vk::raii::PhysicalDevice const& phyDev, vk::QueueFlagBits const& familyBits);
		template <class... Ts>
		bool hasPhysicalDeviceFeatures(vk::raii::PhysicalDevice const& phyDev, vk::StructureChain<Ts...> const& features);
		template <class T>
		bool featureBundleSupported(T const& requested, T const& supported);
		bool hasPhysicalDeviceExtensions(vk::raii::PhysicalDevice const& phyDev, std::vector<const char*> const& extensions);

		uint32_t queueFamilyIndex(vk::raii::PhysicalDevice const& phyDev, vk::raii::SurfaceKHR const& surf, vk::QueueFlagBits const& familyBits);
		std::vector<vk::DeviceQueueCreateInfo> createDeviceQueueCreateInfos(std::vector<std::tuple<vk::QueueFlagBits, uint32_t, float>> const& queuesInfo, std::vector<uint32_t> const& familyIndices);

	public:
		[[nodiscard]] static Context& get();
		template <class... Ts>
		void init(ContextInitInfo<Ts...> const& initInfo);
	};
}
