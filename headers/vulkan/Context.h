#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan_raii.hpp"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <stdexcept>
#include <iostream>
#include <utility>
#include <string>
#include <tuple>

namespace Vulkan {
	class Engine;

	template <class... Ts>
	struct ContextInitInfo {
		int windowWidth = 0;
		int windowHeight = 0;
		const char* appName = nullptr;
		uint32_t apiVersion = 0;
		std::vector<const char*> validationLayers{};
		std::vector<const char*> deviceExtensions{};
		vk::StructureChain<Ts...> deviceFeatures{};
		std::vector<std::tuple<vk::QueueFlagBits, uint32_t, std::vector<float>>> queueFamiliesInfo{};
	};

	class Context {
	private:
		GLFWwindow* window;
		vk::raii::Context context;
		vk::raii::Instance instance;
		vk::raii::SurfaceKHR surface;
		vk::raii::PhysicalDevice physicalDevice;
		vk::raii::Device device;
		std::vector<std::vector<vk::raii::Queue>> queues;

		std::vector<uint32_t> queueFamilyIndices;

		void initWindow(int const& WIDTH, int const& HEIGHT, const char* name);
		void initInstance(uint32_t const& apiVersion, const std::vector<const char*>& validLays);
		void initSurface();
		template <class... Ts>
		void initPhysicalDevice(uint32_t const& apiVersion, std::vector<const char*> const& devExts, vk::StructureChain<Ts...> const& devFeats, std::vector<std::tuple<vk::QueueFlagBits, uint32_t, std::vector<float>>> const& queuesInfo);
		template <class... Ts>
		void initDeviceAndQueues(std::vector<const char*> const& devExts, vk::StructureChain<Ts...> const& devFeats, std::vector<std::tuple<vk::QueueFlagBits, uint32_t, std::vector<float>>> const& queuesInfo);

		// for initInstance
		std::pair<uint32_t, const char**> enumerateGlfwExtensions();
		bool verifyHaveValidationLayers(std::vector<const char*> const& needs);
		bool verifyHaveGlfwExtensions(uint32_t const& needCount, const char**& needs);

		// for initPhysicalDevice
		template <class... Ts>
		std::vector<std::array<std::pair<std::string, uint32_t>, 4>> ratePhysicalDevices(uint32_t const& apiVersion, std::vector<const char*> const& devExts, vk::StructureChain<Ts...> const& devFeats, std::vector<std::tuple<vk::QueueFlagBits, uint32_t, std::vector<float>>> const& queuesInfo);
		uint32_t judgePhysicalDevice(std::array<std::pair<std::string, uint32_t>, 4> rating);
		bool hasMinimumApiVersion(vk::raii::PhysicalDevice const& phyDev, uint32_t const& apiVersion);
		bool hasQueueFamilyQueues(vk::raii::PhysicalDevice const& phyDev, vk::QueueFlagBits const& familyBits, uint32_t const& queueCount);
		template <class... Ts>
		bool hasPhysicalDeviceFeatures(vk::raii::PhysicalDevice const& phyDev, vk::StructureChain<Ts...> const& features);
		template <class T>
		bool featureBundleSupported(T const& requested, T const& supported);
		bool hasPhysicalDeviceExtensions(vk::raii::PhysicalDevice const& phyDev, std::vector<const char*> const& extensions);

		// for initDeviceAndQueues
		uint32_t queueFamilyIndex(vk::raii::PhysicalDevice const& phyDev, vk::raii::SurfaceKHR const& surf, vk::QueueFlagBits const& familyBits);
		std::vector<vk::DeviceQueueCreateInfo> createDeviceQueueCreateInfos(std::vector<std::tuple<vk::QueueFlagBits, uint32_t, std::vector<float>>> const& queuesInfo, std::vector<uint32_t> const& familyIndices);

	public:
		friend class Engine;

		template <class... Ts>
		Context(ContextInitInfo<Ts...> const& initInfo);
		Context(Context&& moveFrom);
		~Context();

		std::vector<uint32_t> getQueueFamilyIndices() const;
	};

	template <class... Ts>
	Context::Context(ContextInitInfo<Ts...> const& initInfo) : window{ nullptr }, context{}, instance{ nullptr }, surface{ nullptr }, physicalDevice{ nullptr }, device{ nullptr }, queues{} {
		initWindow(initInfo.windowWidth, initInfo.windowHeight, initInfo.appName);
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initInstance(initInfo.apiVersion, initInfo.validationLayers);
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initSurface();
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initPhysicalDevice(initInfo.apiVersion, initInfo.deviceExtensions, initInfo.deviceFeatures, initInfo.queueFamiliesInfo);
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initDeviceAndQueues(initInfo.deviceExtensions, initInfo.deviceFeatures, initInfo.queueFamiliesInfo);
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
	}

	template <class... Ts>
	void Context::initPhysicalDevice(uint32_t const& apiVersion, std::vector<const char*> const& devExts, vk::StructureChain<Ts...> const& devFeats, std::vector<std::tuple<vk::QueueFlagBits, uint32_t, std::vector<float>>> const& queuesInfo) {
		std::vector<std::array<std::pair<std::string, uint32_t>, 4>> physicalDeviceRatings = ratePhysicalDevices(apiVersion, devExts, devFeats, queuesInfo);

		bool foundSuitablePhysicalDevice = false;
		for (uint32_t i = 0; i < physicalDeviceRatings.size(); i++) {
			vk::raii::PhysicalDevice phyDev = instance.enumeratePhysicalDevices()[i];
			std::cout << "Rating of " << phyDev.getProperties().deviceName << ": \n" << 
				physicalDeviceRatings[i][0].first << " is " << physicalDeviceRatings[i][0].second << '\n' << 
				physicalDeviceRatings[i][1].first << " is " << physicalDeviceRatings[i][1].second << '\n' << 
				physicalDeviceRatings[i][2].first << " is " << physicalDeviceRatings[i][2].second << '\n' << 
				physicalDeviceRatings[i][3].first << " is " << physicalDeviceRatings[i][3].second << '\n';

			if (judgePhysicalDevice(physicalDeviceRatings[i]) == 4) {
				foundSuitablePhysicalDevice = true;
				physicalDevice = phyDev;
				break;
			}
		}

		if (!foundSuitablePhysicalDevice) {
			throw std::runtime_error("No suitable GPU found");
		}
		else {
			std::cout << "Physical device selection successful: " << physicalDevice.getProperties().deviceName << '\n';
		}
	}

	template <class... Ts>
	void Context::initDeviceAndQueues(std::vector<const char*> const& devExts, vk::StructureChain<Ts...> const& devFeats, std::vector<std::tuple<vk::QueueFlagBits, uint32_t, std::vector<float>>> const& queuesInfo) {
		std::vector<uint32_t> queueFamilyIndices{};
		for (std::tuple<vk::QueueFlagBits, uint32_t, std::vector<float>> const& queueFamily : queuesInfo) {
			queueFamilyIndices.push_back(queueFamilyIndex(physicalDevice, surface, std::get<0>(queueFamily)));
		}
		this->queueFamilyIndices = queueFamilyIndices;

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = createDeviceQueueCreateInfos(queuesInfo, queueFamilyIndices);

		vk::DeviceCreateInfo deviceInfo = {
			.pNext = &devFeats.get<vk::PhysicalDeviceFeatures2>(),
			.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
			.pQueueCreateInfos = queueCreateInfos.data(),
			.enabledExtensionCount = static_cast<uint32_t>(devExts.size()),
			.ppEnabledExtensionNames = devExts.data()
		};

		device = vk::raii::Device(physicalDevice, deviceInfo);

		queues.resize(queuesInfo.size());
		for (uint32_t i = 0; i < queuesInfo.size(); i++) {
			for (uint32_t j = 0; j < std::get<1>(queuesInfo[i]); j++) {
				queues[i].push_back(vk::raii::Queue(device, queueFamilyIndices[i], j));
			}
		}

		std::cout << "Device creation successful with queue families:\n";
		for (uint32_t i = 0; i < queues.size(); i++) {
			std::cout << "\tQueue family " << queueFamilyIndices[i] << " on this GPU with " << queues[i].size() << " queues with priorities {";
			for (float const& f : std::get<2>(queuesInfo[i])) {
				std::cout << f << ", ";
			}
			std::cout << "}\n";
		}
	}

	template <class... Ts>
	std::vector<std::array<std::pair<std::string, uint32_t>, 4>> Context::ratePhysicalDevices(uint32_t const& apiVersion, std::vector<const char*> const& devExts, vk::StructureChain<Ts...> const& devFeats, std::vector<std::tuple<vk::QueueFlagBits, uint32_t, std::vector<float>>> const& queuesInfo) {
		std::vector<std::array<std::pair<std::string, uint32_t>, 4>> physicalDeviceRatings{};
		std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

		for (vk::raii::PhysicalDevice const& phyDev : physicalDevices) {
			std::array<std::pair<std::string, uint32_t>, 4> propertyChecklist = { {
				{"Has minimum api version", 0u},
				{"Has all queue families and sufficient queues for each", 0u},
				{"Has all required extensions", 0u},
				{"Has all required features", 0u}
			}};

			if (hasMinimumApiVersion(phyDev, apiVersion)) {
				propertyChecklist[0].second = 1;
			}

			bool hasAllQueueFamiliesQueues = true;
			for (std::tuple<vk::QueueFlagBits, uint32_t, std::vector<float>> const& queueFamily : queuesInfo) {
				if (!hasQueueFamilyQueues(phyDev, std::get<0>(queueFamily), std::get<1>(queueFamily))) {
					hasAllQueueFamiliesQueues = false;
				}
			}

			if (hasAllQueueFamiliesQueues) {
				propertyChecklist[1].second = 1;
			}

			if (hasPhysicalDeviceExtensions(phyDev, devExts)) {
				propertyChecklist[2].second = 1;
			}

			if (hasPhysicalDeviceFeatures(phyDev, devFeats)) {
				propertyChecklist[3].second = 1;
			}

			physicalDeviceRatings.push_back(propertyChecklist);
		}

		return physicalDeviceRatings;
	}

	template <class... Ts>
	bool Context::hasPhysicalDeviceFeatures(vk::raii::PhysicalDevice const& phyDev, vk::StructureChain<Ts...> const& features) {
		vk::StructureChain<Ts...> availableFeatures = phyDev.getFeatures2<Ts...>();

		return (... && featureBundleSupported(features.get<Ts>(), availableFeatures.get<Ts>()));
	}

	template <class T>
	bool Context::featureBundleSupported(T const& required, T const& available) {
		bool result = true;

		size_t boolOffset = offsetof(T, pNext) + sizeof(void*);
		size_t dataSize = sizeof(T) - boolOffset;

		char const* requiredBoolOffset = reinterpret_cast<char const*>(&required) + boolOffset;
		char const* availableBoolOffset = reinterpret_cast<char const*>(&available) + boolOffset;

		for (uint32_t i = 0; i < dataSize; i += sizeof(VkBool32)) {
			const VkBool32 required = *reinterpret_cast<VkBool32 const*>(requiredBoolOffset + i);
			const VkBool32 available = *reinterpret_cast<VkBool32 const*>(availableBoolOffset + i);

			if (required == VK_TRUE && available == VK_FALSE) {
				result = false;
				break;
			}
			else if (required == VK_TRUE && available == VK_TRUE) {
				std::cout << "Required device feature supported: " << typeid(T).name() << " at location " << boolOffset + i << '\n';
			}
		}

		return result;
	}
}
