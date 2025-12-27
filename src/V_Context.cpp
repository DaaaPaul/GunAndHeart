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
		queues{}
	{

	}

	Context::~Context() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	Context& Context::get() {
		static Context context;
		return context;
	}

	template <class... Ts>
	void Context::init(ContextInitInfo<Ts...> const& initInfo) {
		if (isInitialized) {
			std::cout << "Context already initialized... returning...\n";
			return;
		} else {
			isInitialized = true;
		}

		static vk::StructureChain<Ts...> deviceFeatures = initInfo.deviceFeatures;

		initWindow(initInfo.windowWidth, initInfo.windowHeight, initInfo.appName);
		initInstance(initInfo.apiVersion, initInfo.validationLayers);
		initSurface();
		initPhysicalDevice(initInfo.apiVersion, initInfo.deviceExtensions, initInfo.deviceFeatures, initInfo.queueFamilies);
		initDevice(initInfo.deviceExtensions, initInfo.deviceFeatures, initInfo.queueFamilies);
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

	void Context::initInstance(double const& apiVersion, const std::vector<const char*>& validLays) {
		vk::ApplicationInfo appInfo = {
			.apiVersion = doubleApiVersionToVkApiVersion(apiVersion)
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

		std::cout << "Instance creation successful\n";
	}

	void Context::initSurface() {
		VkSurfaceKHR paperSurface;
		glfwCreateWindowSurface(*instance, window, nullptr, &paperSurface);
		
		surface = vk::raii::SurfaceKHR(instance, paperSurface);

		std::cout << "Surface creation successful\n";
	}

	template <class... Ts>
	void Context::initPhysicalDevice(double const& apiVersion, std::vector<const char*> const& devExts, vk::StructureChain<Ts...> const& devFeats, std::vector<std::tuple<vk::QueueFlagBits, uint32_t, float>> const& queuesInfo) {
		std::vector<std::array<uint32_t, 4>> physicalDeviceRatings = ratePhysicalDevices(apiVersion, devExts, devFeats, queuesInfo);

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

	template <class... Ts>
	void Context::initDevice(std::vector<const char*> const& devExts, vk::StructureChain<Ts...> const& devFeats, std::vector<std::tuple<vk::QueueFlagBits, uint32_t, float>> const& queuesInfo) {
		std::vector<uint32_t> queueFamilyIndices{};
		for(std::tuple<vk::QueueFlagBits, uint32_t, float> const& queueFamily : queuesInfo) {
			queueFamilyIndices.push_back(queueFamilyIndex(physicalDevice, surface, std::get<0>(queuesInfo)));
		}

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos = createDeviceQueueCreateInfos(queuesInfo, queueFamilyIndices);

		vk::DeviceCreateInfo deviceInfo = {
			.pNext = &devFeats.get<vk::PhysicalDeviceFeatures2>(),
			.queueCreateInfoCount = queueCreateInfos.size(),
			.pQueueCreateInfos = queueCreateInfos.data(),
			.enabledExtensionCount = static_cast<uint32_t>(devExts.size()),
			.ppEnabledExtensionNames = devExts.data()
		};

		device = vk::raii::Device(physicalDevice, deviceInfo);

		for(uint32_t i = 0; i < std::tuple_size_v(decltype(queuesInfo)); i++) {
			constexpr i_ = i;

			for (uint32_t j = 0; j < std::get<i_>(queuesInfo); j++) {
				queues[i].push_back(vk::raii::Queue(device, queueFamilyIndices[i], j));
			}
		}

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

	template <class... Ts>
	std::vector<std::array<uint32_t, 4>> Context::ratePhysicalDevices(double const& apiVersion, std::vector<const char*> const& devExts, vk::StructureChain<Ts...> const& devFeats, std::vector<std::tuple<vk::QueueFlagBits, uint32_t, float>> const& queuesInfo) {
		std::vector<std::array<uint32_t, 4>> physicalDeviceRatings{};
		std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

		for(vk::raii::PhysicalDevice const& phyDev : physicalDevices) {
			std::array<uint32_t, 4> propertyChecklist = { 0, 0, 0, 0 };

			if(hasMinimumApiVersion(phyDev, doubleApiVersionToVkApiVersion(apiVersion))) {
				propertyChecklist[0] = 1;
			}

			bool hasAllQueueFamilies = true;
			for(std::tuple<vk::QueueFlagBits, uint32_t, float> const& queueFamily : queuesInfo) {
				if(!hasQueueFamily(phyDev, std::get<0>(queueFamily))) {
					hasAllQueueFamilies = false;
				}
			}

			if (hasAllQueueFamilies) {
				propertyChecklist[1] = 1;
			}

			if (hasPhysicalDeviceExtensions(phyDev, devExts)) {
				propertyChecklist[2] = 1;
			}

			if(hasPhysicalDeviceFeatures(phyDev, devFeats)) {
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

	std::vector<vk::DeviceQueueCreateInfo> Context::createDeviceQueueCreateInfos(std::vector<std::tuple<vk::QueueFlagBits, uint32_t, float>> const& queuesInfo, std::vector<uint32_t> const& familyIndices) {
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};

		std::vector<float> priorities{};
		for(std::tuple<vk::QueueFlagBits, uint32_t, float> const& queueFamily : queuesInfo) {
			priorities.push_back(std::get<2>(queueFamily));
		}

		for(uint32_t i = 0; i < familyIndices.size(); i++) {
			queueCreateInfos.push_back(vk::DeviceQueueCreateInfo{
				.queueFamilyIndex = familyIndices[i],
				.queueCount = std::get<1>(queuesInfo[i]),
				.pQueuePriorities = priorities.data()
			});
		}

		return queueCreateInfos;
	}
}