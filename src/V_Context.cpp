#include "V_Context.h"
#include <cstddef>
#include <limits>

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
		deviceFeatures{ {}, {}, {}, {} }
	{

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
		initInstance(initInfo.apiVersion, initInfo.validationLayers);
		initSurface();
		initPhysicalDevice(initInfo.apiVersion, initInfo.deviceExtensions);
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

	void Context::initInstance(double const& apiVersion, const std::vector<const char*>& VALIDATION) {
		vk::ApplicationInfo appInfo = {
			.apiVersion = doubleApiVersionToVkApiVersion(apiVersion)
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

	void Context::initPhysicalDevice(double const& apiVersion, std::vector<const char*> const& devExts) {
		std::vector<std::array<uint32_t, 4>> physicalDeviceRatings = ratePhysicalDevices(apiVersion, devExts);

		bool foundSuitablePhysicalDevice = false;
		for(uint32_t i = 0; i < physicalDeviceRatings.size(); i++) {
			vk::raii::PhysicalDevice phyDev = instance.enumeratePhysicalDevices()[i];
			std::cout << "Rating of " << phyDev.getProperties().deviceName << ": " << physicalDeviceRatings[i][0] << ' ' << physicalDeviceRatings[i][1] << ' ' << physicalDeviceRatings[i][2] << ' ' << physicalDeviceRatings[i][3] << '\n';

			if(judgePhysicalDevice(physicalDeviceRatings[i]) == 4) {
				foundSuitablePhysicalDevice = true;
				physicalDevice = phyDev;
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
		uint32_t graphicsQueueFamilyIndex = queueFamilyIndex(physicalDevice, surface, vk::QueueFlagBits::eGraphics);
		float arbitraryPriority = 0.5f;
		vk::DeviceQueueCreateInfo graphicsFamilyInfo = {
			.queueFamilyIndex = graphicsQueueFamilyIndex,
			.queueCount = 1,
			.pQueuePriorities = &arbitraryPriority
		};

		vk::DeviceCreateInfo deviceInfo = {
			.pNext = &deviceFeatures.get<vk::PhysicalDeviceFeatures2>(),
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &graphicsFamilyInfo,
			.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size()),
			.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data()
		};

		device = vk::raii::Device(physicalDevice, deviceInfo);
		graphicsQueue = vk::raii::Queue(device, graphicsQueueFamilyIndex, 0);

		std::cout << "Device creation successful\n";
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

	std::vector<std::array<uint32_t, 4>> Context::ratePhysicalDevices(double const& apiVersion, std::vector<const char*> const& devExts) {
		std::vector<std::array<uint32_t, 4>> physicalDeviceRatings{};
		std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

		for(vk::raii::PhysicalDevice const& phyDev : physicalDevices) {
			std::array<uint32_t, 4> propertyChecklist = { 0, 0, 0, 0 };

			if(hasMinimumApiVersion(phyDev, doubleApiVersionToVkApiVersion(apiVersion))) {
				propertyChecklist[0] = 1;
			}

			if(hasQueueFamily(phyDev, vk::QueueFlagBits::eGraphics)) {
				propertyChecklist[1] = 1;
			}

			if (hasPhysicalDeviceExtensions(phyDev, devExts)) {
				propertyChecklist[2] = 1;
			}

			if(hasPhysicalDeviceFeatures(phyDev, deviceFeatures)) {
				propertyChecklist[3] = 1;
			}

			physicalDeviceRatings.push_back(propertyChecklist);
		}

		return physicalDeviceRatings;
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
			} else if (required == VK_TRUE && available == VK_TRUE) {
				std::cout << "Required device feature supported: " << typeid(T).name() << " at location " << boolOffset + i << '\n';
			}
		}
		
		return result;
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
			if ((queueFamilyProperties[i].queueFlags & familyBits) && (phyDev.getSurfaceSupportKHR(i, surf))) {
				familyIndex = i;
				break;
			}
		}

		return familyIndex;
	}
}