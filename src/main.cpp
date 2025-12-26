#include <iostream>
#include "V_Context.h"
#include "V_Engine.h"

int main() {
	try {
		Vulkan::Context& context = Vulkan::Context::get();
		Vulkan::ContextInitInfo contextInfo = {
			.windowWidth = 800,
			.windowHeight = 800,
			.appName = "GunAndHeart",
			.apiVersion = 1.3,
			.validationLayers = {"VK_LAYER_KHRONOS_validation"},
			.deviceExtensions = {
				vk::KHRSwapchainExtensionName,
				vk::KHRSpirv14ExtensionName,
				vk::KHRSynchronization2ExtensionName,
				vk::KHRCreateRenderpass2ExtensionName
			}
		};

		context.init(contextInfo);
	} catch(std::exception const& e) {
		std::cout << e.what() << '\n';
	}
}