#include <iostream>
#include "vulkan/VulkanContext.h"
#include "vulkan/GraphicsContext.h"
#include "vulkan/GraphicsEngine.h"
#include "general/Vertex.h"
#include "general/VertexTransformations.h"

int main() {
	try {
		Vulkan::VulkanContextInitInfo<vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> contextInfo = {
			.windowWidth = 800,
			.windowHeight = 600,
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

		std::vector<General::Vertex> verticies = {
			General::Vertex{.colour = glm::vec3(1.0f, 0.0f, 0.0f), .position = glm::vec2(-0.5f, -0.5f) },
			General::Vertex{ .colour = glm::vec3(0.0f, 1.0f, 0.0f), .position = glm::vec2(0.5f, -0.5f) },
			General::Vertex{ .colour = glm::vec3(0.0f, 0.0f, 1.0f), .position = glm::vec2(0.5f, 0.5f) },
			General::Vertex{ .colour = glm::vec3(1.0f, 1.0f, 0.0f), .position = glm::vec2(-0.5f, 0.5f) }
		};

		Vulkan::GraphicsContextInitInfo graphicsContextInfo = {
			.scFormat = vk::SurfaceFormatKHR(vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear),
			.scImageCount = 2,
			.scPresentMode = vk::PresentModeKHR::eMailbox,
			.scImageUsage = vk::ImageUsageFlagBits::eColorAttachment,
			.scImageViewAspect = vk::ImageAspectFlagBits::eColor,
			.scImageSharingMode = vk::SharingMode::eExclusive,
			.scQueueFamilyAccessorCount = 1,
			.scQueueFamilyAccessorIndiceList = context.getQueueFamilyIndices().data(),
			.scPreTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity,
			
			.descriptorSetLayoutBindings = { General::VertexTransformations::getDescriptorSetLayoutBinding(0, 1) },
			.uniformBufferInfo = { 2, sizeof(General::VertexTransformations), vk::SharingMode::eExclusive },
			
			.gpShaderStageInfos = {
				{vk::ShaderStageFlagBits::eVertex, "shaders/shader.spv", "vertexShader"},
				{vk::ShaderStageFlagBits::eFragment, "shaders/shader.spv", "fragmentShader"}
			},
			.gpVertexInputInfo = {
				General::Vertex::getVertexInputBindingDescription(),
				General::Vertex::getVertexInputAttributeDescription()
			},
			.gpInputAssemblyInfo = {
				vk::PrimitiveTopology::eTriangleList,
				false
			},
			.gpViewportStateInfo = {
				{0.0f, 0.0f, 800.0f, 800.0f, 0.0f, 1.0f},
				{0, 0, 800, 800}
			},
			.gpRasterizationInfo = {
				false,
				false,
				vk::PolygonMode::eFill,
				vk::CullModeFlagBits::eNone,
				vk::FrontFace::eClockwise,
				false,
				1.0f,
				0.0f,
				1.0f,
				1.0f
			},
			.gpColourBlendingInfo = {
				{
					std::make_tuple(
						false,
						vk::BlendFactor::eOne,
						vk::BlendFactor::eOne,
						vk::BlendOp::eAdd,
						vk::BlendFactor::eOne,
						vk::BlendFactor::eOne,
						vk::BlendOp::eAdd,
						vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
					)
				},
				std::make_tuple(
					false,
					vk::LogicOp::eClear,
					std::array<float, 4>{ 1.0f, 1.0f, 1.0f, 1.0f }
				)
			},
			.dynamicStates = {
				vk::DynamicState::eViewport,
				vk::DynamicState::eScissor
			},
			.verticiesBufferInfo = {
				vk::SharingMode::eExclusive,
				verticies
			},
			.indexBufferData = {
				0, 1, 2,
				0, 2, 3
			}
		};
		Vulkan::GraphicsContext graphicsContext(std::move(context), graphicsContextInfo);

		Vulkan::GraphicsEngineInitInfo graphicsEngineInfo = {
			.commandPoolsInfos = {
				{vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient, graphicsContext.getContext().getQueueFamilyIndices()[0]}
			},
			.commandBuffersInfos = {
				0, vk::CommandBufferLevel::ePrimary, 2
			},
			.framesInFlightCount = 2
		};

		Vulkan::GraphicsEngine graphicsEngine(std::move(graphicsContext), graphicsEngineInfo);
		graphicsEngine.runLoop();
	} catch(std::exception const& e) {
		std::cout << e.what() << '\n';
	}
}