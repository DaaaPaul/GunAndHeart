#include "V_Context.h"
#include <cstddef>
#include <limits>

namespace Vulkan {
	Context::~Context() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void Context::initWindow(int const& WIDTH, int const& HEIGHT, const char* name) {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(WIDTH, HEIGHT, name, nullptr, nullptr);

		if (window == NULL) {
			throw std::runtime_error("Window creation failure");
		} else {
			std::cout << "Window creation successful with parameters {WIDTH: " << WIDTH << "} {HEIGHT: " << HEIGHT << "} {NAME: " << name << "}\n";
		}
	}

	void Context::initInstance(uint32_t const& apiVersion, const std::vector<const char*>& validLays) {
		vk::ApplicationInfo appInfo = {
			.apiVersion = apiVersion
		};

		std::pair<uint32_t, const char**> glfwExtensionInfo = enumerateGlfwExtensions();

		vk::InstanceCreateInfo instanceInfo = {
			.pApplicationInfo = &appInfo,
			.enabledExtensionCount = glfwExtensionInfo.first,
			.ppEnabledExtensionNames = glfwExtensionInfo.second
		};

		if(!validLays.empty()) {
			instanceInfo.enabledLayerCount = static_cast<uint32_t>(validLays.size());
			instanceInfo.ppEnabledLayerNames = validLays.data();
		}

		instance = vk::raii::Instance(context, instanceInfo);

		std::cout << "Instance creation successful with parameters {API VERSION: " << apiVersion << "} {VALIDATION LAYERS: ";
		for(const char* lay : validLays) {
			std::cout << lay << ", ";
		}
		std::cout << "}\n";
	}

	void Context::initSurface() {
		VkSurfaceKHR paperSurface;
		glfwCreateWindowSurface(*instance, window, nullptr, &paperSurface);
		
		surface = vk::raii::SurfaceKHR(instance, paperSurface);

		std::cout << "Surface creation successful\n";
	}

	std::pair<uint32_t, const char**> Context::enumerateGlfwExtensions() {
		uint32_t requiredExtensionsCount = 0;
		const char** requiredExtensions = glfwGetRequiredInstanceExtensions(&requiredExtensionsCount);

		if(!verifyHaveGlfwExtensions(requiredExtensionsCount, requiredExtensions)) {
			throw std::runtime_error("Required extension by glfw not supported");
		}

		return { requiredExtensionsCount, requiredExtensions };
	}

	bool Context::verifyHaveGlfwExtensions(uint32_t const& needCount, const char**& needs) {
		bool haveGlfwExtensions = true;

		std::vector<vk::ExtensionProperties> extensionProperties = context.enumerateInstanceExtensionProperties();

		bool extensionFound = false;
		for (uint32_t i = 0; i < needCount; ++i) {
			extensionFound = false;

			for (vk::ExtensionProperties const& property : extensionProperties) {
				if (strcmp(property.extensionName, needs[i]) == 0) {
					extensionFound = true;
					break;
				}
			}

			if (!extensionFound) {
				haveGlfwExtensions = false;
			} else {
				std::cout << "Required extension by glfw supported:" << needs[i] << '\n';
			}
		}

		return haveGlfwExtensions;
	}

	uint32_t Context::judgePhysicalDevice(std::array<uint32_t, 4> rating) {
		uint32_t judgement = 0;

		for(uint32_t const& value : rating) {
			judgement += value;
		}

		return judgement;
	}

	bool Context::hasMinimumApiVersion(vk::raii::PhysicalDevice const& phyDev, uint32_t const& apiVersion) {
		return phyDev.getProperties().apiVersion >= apiVersion;
	}

	bool Context::hasQueueFamilyQueues(vk::raii::PhysicalDevice const& phyDev, vk::QueueFlagBits const& familyBit, uint32_t const& queueCount) {
		bool hasNecessary = false;
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = phyDev.getQueueFamilyProperties();

		for (vk::QueueFamilyProperties const& queueFamily : queueFamilyProperties) {
			if ((queueFamily.queueFlags & familyBit) && (queueFamily.queueCount >= queueCount)) {
				hasNecessary = true;
				break;
			}
		}
		
		return hasNecessary;
	}

	bool Context::hasPhysicalDeviceExtensions(vk::raii::PhysicalDevice const& phyDev, std::vector<const char*> const& extensions) {
		std::vector<vk::ExtensionProperties> extensionProperties = phyDev.enumerateDeviceExtensionProperties();
		bool foundAllExtensions = true;
		
		bool foundExtension = false;
		for (const char* requiredExtension : extensions) {
			foundExtension = false;

			for (vk::ExtensionProperties const& property : extensionProperties) {
				if (strcmp(property.extensionName, requiredExtension) == 0) {
					std::cout << "Required physical device extension supported:" << requiredExtension << '\n';
					foundExtension = true;
					break;
				}
			}

			if (!foundExtension) {
				foundAllExtensions = false;
			}
		}

		return foundAllExtensions;
	}

	uint32_t Context::queueFamilyIndex(vk::raii::PhysicalDevice const& phyDev, vk::raii::SurfaceKHR const& surf, vk::QueueFlagBits const& familyBits) {
		uint32_t familyIndex = std::numeric_limits<uint32_t>::max();
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = phyDev.getQueueFamilyProperties();

		for(uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
			if (queueFamilyProperties[i].queueFlags & familyBits) {
				if(familyBits & vk::QueueFlagBits::eGraphics) {
					if(phyDev.getSurfaceSupportKHR(i, surf)) {
						familyIndex = i;
						break;
					}
				} else {
					familyIndex = i;
					break;
				}
			}
		}

		return familyIndex;
	}

	std::vector<vk::DeviceQueueCreateInfo> Context::createDeviceQueueCreateInfos(std::vector<std::tuple<vk::QueueFlagBits, uint32_t, std::vector<float>>> const& queuesInfo, std::vector<uint32_t> const& familyIndices) {
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};

		for(uint32_t i = 0; i < queuesInfo.size(); i++) {
			queueCreateInfos.push_back(vk::DeviceQueueCreateInfo{
				.queueFamilyIndex = familyIndices[i],
				.queueCount = std::get<1>(queuesInfo[i]),
				.pQueuePriorities = std::get<2>(queuesInfo[i]).data()
			});
		}

		return queueCreateInfos;	
	}
}