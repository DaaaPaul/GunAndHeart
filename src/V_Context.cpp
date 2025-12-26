#include "V_Context.h"

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
		self{ *this }
		{ }

	Context::~Context() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	Context& Context::get() {
		static Context context;
		return context;
	}

	void Context::init(ContextInitInfo const& initInfo) {
		if (isInitialized) std::cout << "Context already initialized... returning...\n";
		else isInitialized = true;

		initWindow(initInfo.windowWidth, initInfo.windowHeight, initInfo.appName);
		initInstance(initInfo.validationLayers);
		initSurface();
		initPhysicalDevice();
		initDevice();
	}

	void Context::initWindow(int const& WIDTH, int const& HEIGHT, const char* name) {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(WIDTH, HEIGHT, name, nullptr, nullptr);

		if (window == NULL) throw std::runtime_error("Window creation failure");
		else std::cout << "Window creation successful\n";
	}

	void Context::initInstance(const std::vector<const char*>& VALIDATION) {
		const vk::ApplicationInfo appInfo = {
			.apiVersion = vk::ApiVersion14
		};

		const std::pair<uint32_t, const char**> glfwExtensionInfo = enumerateGlfwExtensions();

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

	void Context::initPhysicalDevice() {
	
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
}