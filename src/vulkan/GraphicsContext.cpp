#include "vulkan/GraphicsContext.h"
#include <fstream>

namespace Vulkan {
	GraphicsContext::GraphicsContext(VulkanContext&& context, GraphicsContextInitInfo const& initInfo) : context(std::move(context)), swapchain{ nullptr }, scImageViews{}, graphicsPipeline{ nullptr } {
		initSwapchainAndImageViews(initInfo.scFormat, initInfo.scImageCount, initInfo.scPresentMode, initInfo.scImageUsage, initInfo.scImageViewAspect, initInfo.scImageSharingMode, initInfo.scQueueFamilyAccessorCount, initInfo.scQueueFamilyAccessorIndiceList, initInfo.scPreTransform);
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initGraphicsPipeline(initInfo.gpSprivModuleInfos, initInfo.gpInputAssemblyInfo, initInfo.gpViewportStateInfo, initInfo.gpRasterizationInfo, initInfo.gpColourBlendingInfo, initInfo.dynamicStates);
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
	}

	void GraphicsContext::initSwapchainAndImageViews(vk::SurfaceFormatKHR const& desiredFormat, uint32_t const& desiredImageCount, vk::PresentModeKHR const& desiredPresentMode, vk::ImageUsageFlagBits const& imageUsage, vk::ImageAspectFlagBits const& imageViewAspect, vk::SharingMode const& sharingMode, uint32_t const& queueFamilyAccessorCount, uint32_t* queueFamilyAccessorIndiceList, vk::SurfaceTransformFlagBitsKHR const& preTransform) {
		vk::Extent2D extent = getSurfaceExtent();
		vk::SurfaceFormatKHR format = getScFormat(desiredFormat);
		uint32_t imageCount = getScImageCount(desiredImageCount);
		vk::PresentModeKHR presentMode = getScPresentMode(desiredPresentMode);

		if (format == vk::SurfaceFormatKHR{}) {
			throw std::runtime_error("Desired swapchain format not supported");
		}
		if(imageCount == 0xFFFFFFFF) {
			throw std::runtime_error("Desired swapchain image count not supported");
		}
		if (presentMode == vk::PresentModeKHR{}) {
			throw std::runtime_error("Desired swapchain present mode not supported");
		}

		vk::SwapchainCreateInfoKHR swapchainInfo = {
			.surface = context.surface,
			.minImageCount = imageCount,
			.imageFormat = format.format,
			.imageColorSpace = format.colorSpace,
			.imageExtent = extent,
			.imageArrayLayers = 1,
			.imageUsage = imageUsage,
			.imageSharingMode = sharingMode,
			.queueFamilyIndexCount = queueFamilyAccessorCount,
			.pQueueFamilyIndices = queueFamilyAccessorIndiceList,
			.preTransform = preTransform,
			.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
			.presentMode = presentMode,
			.clipped = true
		};

		swapchain = vk::raii::SwapchainKHR(context.device, swapchainInfo);
		std::cout << "Created swapchain\n";

		std::vector<vk::Image> scImages = swapchain.getImages();

		vk::ImageViewCreateInfo imageViewCreateInfo = {
			.image = {},
			.viewType = vk::ImageViewType::e2D,
			.format = format.format,
			.subresourceRange = vk::ImageSubresourceRange(imageViewAspect, 0, 1, 0, 1)
		};

		for(vk::Image const& scIm : scImages) {
			imageViewCreateInfo.image = scIm;
			scImageViews.push_back(vk::raii::ImageView(context.device, imageViewCreateInfo));
		}

		std::cout << "Created " << scImageViews.size() << " image views for the swapchain\n";
	}

	void GraphicsContext::initGraphicsPipeline(const char* const& sprivInfos, std::tuple<vk::PrimitiveTopology, bool> const& inAssemInfo, std::tuple<std::array<float, 6>, std::array<uint32_t, 4>> const& viewInfo, std::tuple<bool, bool, vk::PolygonMode, vk::CullModeFlagBits, vk::FrontFace, bool, float, float, float, float> const& rasInfo, std::tuple<std::vector<std::tuple<bool, vk::BlendFactor, vk::BlendFactor, vk::BlendOp, vk::BlendFactor, vk::BlendFactor, vk::BlendOp, vk::ColorComponentFlags>>, std::tuple<bool, vk::LogicOp, std::array<float, 4>>> const& cBlendInfo, std::vector<vk::DynamicState> const& dyInfo) {
		std::vector<char> buf = fileBytes("shaders/shader.spv");
		vk::raii::ShaderModule shaderModule = getShaderModule(buf);
		vk::PipelineShaderStageCreateInfo yes1 = {
			.stage = vk::ShaderStageFlagBits::eVertex,
			.module = shaderModule,
			.pName = "vertexShader"
		};
		vk::PipelineShaderStageCreateInfo yes2 = {
			.stage = vk::ShaderStageFlagBits::eFragment,
			.module = shaderModule,
			.pName = "fragmentShader"
		};
		std::vector<vk::PipelineShaderStageCreateInfo> shaderCreateInfo = { yes1, yes2 };

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo; // HARD CODED NANA

		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {
			.topology = std::get<0>(inAssemInfo),
			.primitiveRestartEnable = static_cast<uint32_t>(std::get<1>(inAssemInfo))
		};

		vk::Viewport viewport(std::get<0>(viewInfo)[0], std::get<0>(viewInfo)[1], std::get<0>(viewInfo)[2], std::get<0>(viewInfo)[3], std::get<0>(viewInfo)[4], std::get<0>(viewInfo)[5]);
		vk::Rect2D scissor(vk::Offset2D(std::get<1>(viewInfo)[0], std::get<1>(viewInfo)[1]), vk::Extent2D(std::get<1>(viewInfo)[2], std::get<1>(viewInfo)[3]));
		vk::PipelineViewportStateCreateInfo viewportScissorInfo = {
			.viewportCount = 1,
			.pViewports = &viewport,
			.scissorCount = 1,
			.pScissors = &scissor
		};

		vk::PipelineRasterizationStateCreateInfo rasterizationInfo = {
			.depthClampEnable = std::get<0>(rasInfo),
			.rasterizerDiscardEnable = std::get<1>(rasInfo),
			.polygonMode = std::get<2>(rasInfo),
			.cullMode = std::get<3>(rasInfo),
			.frontFace = std::get<4>(rasInfo),
			.depthBiasEnable = std::get<5>(rasInfo),
			.depthBiasConstantFactor = std::get<6>(rasInfo),
			.depthBiasClamp = std::get<7>(rasInfo),
			.depthBiasSlopeFactor = std::get<8>(rasInfo),
			.lineWidth = std::get<9>(rasInfo)
		};

		// HARD CODED NANA
		vk::PipelineMultisampleStateCreateInfo multisampling{ .rasterizationSamples = vk::SampleCountFlagBits::e1, .sampleShadingEnable = vk::False };

		std::vector<vk::PipelineColorBlendAttachmentState> attachmentInfos{};
		for(std::tuple<bool, vk::BlendFactor, vk::BlendFactor, vk::BlendOp, vk::BlendFactor, vk::BlendFactor, vk::BlendOp, vk::ColorComponentFlags> const& attInfo : std::get<0>(cBlendInfo)) {
			attachmentInfos.emplace_back(std::get<0>(attInfo), std::get<1>(attInfo), std::get<2>(attInfo), std::get<3>(attInfo), std::get<4>(attInfo), std::get<5>(attInfo), std::get<6>(attInfo), std::get<7>(attInfo));
		}
		vk::PipelineColorBlendStateCreateInfo colorBlendInfo = {
			.logicOpEnable = std::get<0>(std::get<1>(cBlendInfo)),
			.logicOp = std::get<1>(std::get<1>(cBlendInfo)),
			.attachmentCount = static_cast<uint32_t>(attachmentInfos.size()),
			.pAttachments = attachmentInfos.data(),
			.blendConstants = std::get<2>(std::get<1>(cBlendInfo))
		};

		vk::PipelineDynamicStateCreateInfo dynamicStateInfo = {
			.dynamicStateCount = static_cast<uint32_t>(dyInfo.size()),
			.pDynamicStates = dyInfo.data()
		};

		// HARD CODED NANA
		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{ .setLayoutCount = 0, .pushConstantRangeCount = 0 };
		vk::raii::PipelineLayout pipelineLayout = vk::raii::PipelineLayout(context.device, pipelineLayoutInfo);

		// HARD CODED NANA
		vk::Format tempformat = vk::Format::eR8G8B8A8Srgb;
		vk::PipelineRenderingCreateInfo renderingAttachmentInfo = {
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &tempformat
		};

		vk::GraphicsPipelineCreateInfo graphicsPipelineInfo = {
			.pNext = &renderingAttachmentInfo,
			.stageCount = 2,
			.pStages = shaderCreateInfo.data(),
			.pVertexInputState = &vertexInputInfo,
			.pInputAssemblyState = &inputAssemblyInfo,
			.pViewportState = &viewportScissorInfo,
			.pRasterizationState = &rasterizationInfo,
			.pMultisampleState = &multisampling,
			.pColorBlendState = &colorBlendInfo,
			.pDynamicState = &dynamicStateInfo,
			.layout = pipelineLayout,
			.renderPass = nullptr
		};

		graphicsPipeline = vk::raii::Pipeline(context.device, nullptr, graphicsPipelineInfo);
		std::cout << "Created graphics pipeline\n";
	}

	// already decided by how large the window is
	vk::Extent2D GraphicsContext::getSurfaceExtent() {
		vk::SurfaceCapabilitiesKHR surfaceCapabilities = context.physicalDevice.getSurfaceCapabilitiesKHR(context.surface);
		vk::Extent2D selectedExtent{};

		if(surfaceCapabilities.currentExtent.width == 0xFFFFFFFF && surfaceCapabilities.currentExtent.height) {
			int width = 0;
			int height = 0;

			glfwGetFramebufferSize(context.window, &width, &height);

			selectedExtent = vk::Extent2D(
				std::clamp<uint32_t>(width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width),
				std::clamp<uint32_t>(height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height)
			);
		} else {
			selectedExtent = surfaceCapabilities.currentExtent;
		}

		return selectedExtent;
	}

	// returns vk::SurfaceFormatKHR{} if desiredFormat is not found in compatible formats
	vk::SurfaceFormatKHR GraphicsContext::getScFormat(vk::SurfaceFormatKHR const& desiredFormat) {
		std::vector<vk::SurfaceFormatKHR> compatibleSurfaceFormats = context.physicalDevice.getSurfaceFormatsKHR(context.surface);
		vk::SurfaceFormatKHR selectedFormat{};

		for(vk::SurfaceFormatKHR const& compFormat : compatibleSurfaceFormats) {
			if(compFormat == desiredFormat) {
				selectedFormat = compFormat;
			}
		}

		return selectedFormat;
	}

	// returns 0xFFFFFFFF if desired image count is outside of possible range
	uint32_t GraphicsContext::getScImageCount(uint32_t const& desiredImageCount) {
		vk::SurfaceCapabilitiesKHR surfaceCapabilities = context.physicalDevice.getSurfaceCapabilitiesKHR(context.surface);
		uint32_t selectedImageCount = 0xFFFFFFFF;
		uint32_t minImages = surfaceCapabilities.minImageCount;
		uint32_t maxImages = surfaceCapabilities.maxImageCount;

		if(desiredImageCount >= minImages && desiredImageCount <= maxImages) {
			selectedImageCount = desiredImageCount;
		}

		return selectedImageCount;
	}

	// returns vk::PresentModeKHR{} if desiredPresentMode is not found in compatible present modes
	vk::PresentModeKHR GraphicsContext::getScPresentMode(vk::PresentModeKHR const& desiredPresentMode) {
		std::vector<vk::PresentModeKHR> compatiblePresentModes = context.physicalDevice.getSurfacePresentModesKHR(context.surface);
		vk::PresentModeKHR selectedPresentMode{};

		for(vk::PresentModeKHR const& compPreMode : compatiblePresentModes) {
			if(compPreMode == desiredPresentMode) {
				selectedPresentMode = compPreMode;
			}
		}

		return selectedPresentMode;
	}

	vk::raii::ShaderModule GraphicsContext::getShaderModule(std::vector<char> const& bytes) {
		vk::ShaderModuleCreateInfo shaderModuleInfo = {
			.codeSize = bytes.size() * sizeof(char),
			.pCode = reinterpret_cast<const uint32_t*>(bytes.data())
		};
		vk::raii::ShaderModule shaderModule = context.device.createShaderModule(shaderModuleInfo);

		return shaderModule;
	}

	std::vector<char> GraphicsContext::fileBytes(std::string const& path) {
		std::ifstream fileInput(path, std::ios::ate | std::ios::binary);
		if(!fileInput.good()) {
			throw std::runtime_error("Failure reading file at " + path);
		}

		std::vector<char> bytes(fileInput.tellg());
		fileInput.seekg(0, std::ios::beg);
		fileInput.read(bytes.data(), static_cast<std::streamsize>(bytes.size()));
		fileInput.close();

		return bytes;
	}
}