#include <iostream>
#include "vulkan/VulkanContext.h"
#include "vulkan/GraphicsContext.h"

int main() {
	try {
		Vulkan::VulkanContextInitInfo<vk::PhysicalDeviceFeatures2,
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
			.queueFamiliesInfo = {
				{vk::QueueFlagBits::eGraphics, 1, {0.5f}}
			}
		};
		Vulkan::VulkanContext context(contextInfo);

		Vulkan::GraphicsContextInitInfo engineInfo = {
			.swapchainFormat = vk::SurfaceFormatKHR(vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear),
			.swapchainImageCount = 2,
			.swapchainPresentMode = vk::PresentModeKHR::eMailbox,
			.swapchainImageUsage = vk::ImageUsageFlagBits::eColorAttachment,
			.imageViewAspect = vk::ImageAspectFlagBits::eColor,
			.swapchainImageSharingMode = vk::SharingMode::eExclusive,
			.swapchainQueueFamilyAccessorCount = 1,
			.swapchainQueueFamilyAccessorIndiceList = context.getQueueFamilyIndices().data(),
			.swapchainPreTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity,
			.pipelineSprivModuleInfos = {
				{vk::ShaderStageFlagBits::eVertex, "shaders/shader.spv", "vertexShader"},
				{vk::ShaderStageFlagBits::eFragment, "shaders/shader.spv", "fragmentShader"}
			}
		};
		Vulkan::GraphicsContext graphicsContext(std::move(context), engineInfo);
	} catch(std::exception const& e) {
		std::cout << e.what() << '\n';
	}
}