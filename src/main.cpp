#include <iostream>
#include "V_Context.h"
#include "V_Engine.h"

int main() {
	try {
		Vulkan::Context& context = Vulkan::Context::get();

		Vulkan::ContextInitInfo<vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> contextInfo = {
			.windowWidth = 800,
			.windowHeight = 800,
			.appName = "GunAndHeart",
			.apiVersion = vk::ApiVersion13,
			.validationLayers = {"VK_LAYER_KHRONOS_validation"},
			.deviceExtensions = {
				vk::KHRSwapchainExtensionName,
				vk::KHRSpirv14ExtensionName,
				vk::KHRSynchronization2ExtensionName,
				vk::KHRCreateRenderpass2ExtensionName
			},
			.deviceFeatures = 
				vk::StructureChain<vk::PhysicalDeviceFeatures2,
				vk::PhysicalDeviceVulkan11Features,
				vk::PhysicalDeviceVulkan13Features,
				vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> {
					{},
					{.shaderDrawParameters = true },
					{.synchronization2 = true, .dynamicRendering = true },
					{.extendedDynamicState = true }
				},
			.queueFamilies = {
				{vk::QueueFlagBits::eGraphics, 1, {0.5f}}
			}
		};

		context.init(contextInfo);
	} catch(std::exception const& e) {
		std::cout << e.what() << '\n';
	}
}