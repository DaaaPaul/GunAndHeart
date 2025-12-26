#include "V_Context.h"
#include <cstddef>

namespace Vulkan {
	Context::Context() : 
		isInitialized{ false },
		window{ nullptr }, 
		context{}, 
		instance{ nullptr }, 
		surface{ nullptr }, 
		physicalDevice{ nullptr }, 
		device{ nullptr }, 
		graphicsQueue{ nullptr },
		WINDOW_WIDTH{ 0 },
		WINDOW_HEIGHT{ 0 },
		windowName{ nullptr },
		VALIDATION_LAYERS{},
		DEVICE_EXTENSIONS{},
		deviceFeatures{ {}, {}, {}, {} },
		self{ *this } {

		deviceFeatures.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters = true;
		deviceFeatures.get<vk::PhysicalDeviceVulkan13Features>().synchronization2 = true;
		deviceFeatures.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering = true;
		deviceFeatures.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState = true;

	}

	Context::~Context() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	Context& Context::get() {
		static Context context;
		return context;
	}

	void Context::init(ContextInitInfo const& initInfo) {
		if (isInitialized) {
			std::cout << "Context already initialized... returning...\n";
			return;
		} else {
			isInitialized = true;
		}

		initWindow(initInfo.windowWidth, initInfo.windowHeight, initInfo.appName);
		initInstance(initInfo.validationLayers);
		initSurface();
		initPhysicalDevice(initInfo.deviceExtensions);
		initDevice();
	}

	void Context::initWindow(int const& WIDTH, int const& HEIGHT, const char* name) {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(WIDTH, HEIGHT, name, nullptr, nullptr);

		if (window == NULL) {
			throw std::runtime_error("Window creation failure");
		} else {
			std::cout << "Window creation successful\n";
		}
	}

	void Context::initInstance(const std::vector<const char*>& VALIDATION) {
		vk::ApplicationInfo appInfo = {
			.apiVersion = vk::ApiVersion14
		};

		std::pair<uint32_t, const char**> glfwExtensionInfo = enumerateGlfwExtensions();

		vk::InstanceCreateInfo instanceInfo = {
			.pApplicationInfo = &appInfo,
			.enabledExtensionCount = glfwExtensionInfo.first,
			.ppEnabledExtensionNames = glfwExtensionInfo.second
		};

		if(!VALIDATION.empty()) {
			instanceInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION.size());
			instanceInfo.ppEnabledLayerNames = VALIDATION.data();
		}

		instance = vk::raii::Instance(context, instanceInfo);

		std::cout << "Instance creation successful\n";
	}

	void Context::initSurface() {
		VkSurfaceKHR paperSurface;
		glfwCreateWindowSurface(*instance, window, nullptr, &paperSurface);
		
		surface = vk::raii::SurfaceKHR(instance, paperSurface);

		std::cout << "Surface creation successful\n";
	}

	void Context::initPhysicalDevice(std::vector<const char*> const& devExts) {
		std::vector<std::array<uint32_t, 4>> physicalDeviceProperties = enumeratePhysicalDeviceProperties(devExts);

		bool foundSuitablePhysicalDevice = false;
		for(uint32_t i = 0; i < physicalDeviceProperties.size(); i++) {
			std::cout << physicalDeviceProperties[i][0] << '\n';
			std::cout << physicalDeviceProperties[i][1] << '\n';
			std::cout << physicalDeviceProperties[i][2] << '\n';
			std::cout << physicalDeviceProperties[i][3] << '\n';

			if (physicalDeviceProperties[i][0] && physicalDeviceProperties[i][1] && physicalDeviceProperties[i][2] && physicalDeviceProperties[i][3]) {
				physicalDevice = instance.enumeratePhysicalDevices()[i];
				foundSuitablePhysicalDevice = true;
				break;
			}
		}

		if(!foundSuitablePhysicalDevice) {
			throw std::runtime_error("No suitable GPU found");
		} else {
			std::cout << "Physical device selection sucessful: " << physicalDevice.getProperties().deviceName << '\n';
		}
	}

	void Context::initDevice() {
	
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

	std::vector<std::array<uint32_t, 4>> Context::enumeratePhysicalDeviceProperties(std::vector<const char*> const& devExts) {
		std::vector<std::array<uint32_t, 4>> physicalDeviceProperties{};
		std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

		for(vk::raii::PhysicalDevice const& phyDev : physicalDevices) {
			std::array<uint32_t, 4> propertyChecklist = { 0, 0, 0, 0 };

			if(hasMinimumApiVersion(phyDev, VK_VERSION_1_3)) {
				propertyChecklist[0] = 1;
			}

			if(hasQueueFamily(phyDev, vk::QueueFlagBits::eGraphics)) {
				propertyChecklist[1] = 1;
			}

			if(hasPhysicalDeviceFeatures(phyDev, deviceFeatures)) {
				propertyChecklist[2] = 1;
			}

			if(hasPhysicalDeviceExtensions(phyDev, devExts)) {
				propertyChecklist[3] = 1;
			}

			physicalDeviceProperties.push_back(propertyChecklist);
		}

		return physicalDeviceProperties;
	}

	bool Context::hasMinimumApiVersion(vk::raii::PhysicalDevice const& phyDev, int const& apiVersion) {
		return phyDev.getProperties().apiVersion >= static_cast<uint32_t>(apiVersion);
	}

	bool Context::hasQueueFamily(vk::raii::PhysicalDevice const& phyDev, vk::QueueFlagBits const& familyBit) {
		bool foundFamily = false;
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = phyDev.getQueueFamilyProperties();

		for (vk::QueueFamilyProperties const& queueFamily : queueFamilyProperties) {
			if (queueFamily.queueFlags & familyBit) {
				foundFamily = true;
				break;
			}
		}
		
		return foundFamily;
	}

	template <class... Ts>
	bool Context::hasPhysicalDeviceFeatures(vk::raii::PhysicalDevice const& phyDev, vk::StructureChain<Ts...> const& features) {
		vk::StructureChain<Ts...> availableFeatures = phyDev.getFeatures2<Ts...>();

		return (... && isFeaturesBundleSupported(features.get<Ts>(), availableFeatures.get<Ts>()));
	}

	template <class T>
	bool Context::isFeaturesBundleSupported(T const& requested, T const& supported) {
		size_t offset = offsetof(T, pNext) + sizeof(void*);

		char const* reqBytes = reinterpret_cast<char const*>(&requested) + offset;
		char const* supBytes = reinterpret_cast<char const*>(&supported) + offset;

		size_t dataSize = sizeof(T) - offset;

		for (size_t i = 0; i < dataSize; i += sizeof(VkBool32)) {
			VkBool32 reqVal = *reinterpret_cast<VkBool32 const*>(reqBytes + i);
			VkBool32 supVal = *reinterpret_cast<VkBool32 const*>(supBytes + i);

			if ((supVal & reqVal) != reqVal) {
				return false;
			}
		}

		return true;
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
}