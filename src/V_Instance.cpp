#include "V_Instance.h"
#include <vector>
#include <stdexcept>
#include <iostream>

namespace Vulkan {
	bool Instance::hasGlfwExtensions() {
		bool foundAll = true;

		glfwInit();

		uint32_t requiredExtensionsCount = 0;
		const char** requiredExtensions = glfwGetRequiredInstanceExtensions(&requiredExtensionsCount);

		std::vector<vk::ExtensionProperties> availableExtensions = context.enumerateInstanceExtensionProperties();

		bool found = false;
		for(uint32_t i = 0; i < requiredExtensionsCount; ++i) {
			found = false;

			for(vk::ExtensionProperties const& availableExtension : availableExtensions) {
				if(strcmp(availableExtension.extensionName, requiredExtensions[i]) == 0) {
					found = true;
					break;
				}
			}

			if (!found) {
				foundAll = false;
			} else {
				std::cout << "Found required GLFW extension: " << requiredExtensions[i] << '\n';
			}
		}

		return foundAll;
	}

	Instance::Instance() : context{}, instance{ nullptr } {
		hasGlfwExtensions();
	}
}