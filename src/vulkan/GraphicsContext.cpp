#include "vulkan/GraphicsContext.h"
#include <fstream>

namespace Vulkan {
	GraphicsContext::GraphicsContext(VulkanContext&& context, GraphicsContextInitInfo const& initInfo) : context(std::move(context)), swapchain{ nullptr }, scImageViews{}, graphicsPipeline{ nullptr }, verticiesBuffer{ nullptr }, verticiesBufferMemory{ nullptr }, indicesBuffer{ nullptr }, indicesBufferMemory{ nullptr }, verticiesCount{}, indicesCount{}, descriptorSetLayout{ nullptr }, uniformBuffers{}, uniformBuffersMemory{}, uniformBuffersAddresses{}, savedScConfigInfo{ initInfo.scFormat, initInfo.scImageCount, initInfo.scPresentMode, initInfo.scImageUsage, initInfo.scImageViewAspect, initInfo.scImageSharingMode, initInfo.scQueueFamilyAccessorCount, initInfo.scQueueFamilyAccessorIndiceList, initInfo.scPreTransform } {
		initSwapchainAndImageViews(initInfo.scFormat, initInfo.scImageCount, initInfo.scPresentMode, initInfo.scImageUsage, initInfo.scImageViewAspect, initInfo.scImageSharingMode, initInfo.scQueueFamilyAccessorCount, initInfo.scQueueFamilyAccessorIndiceList, initInfo.scPreTransform);
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initDescriptorSetLayout(initInfo.descriptorSetLayoutBindings);
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initUniformBuffers(initInfo.uniformBufferInfo);
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initGraphicsPipeline(initInfo.gpShaderStageInfos, initInfo.gpVertexInputInfo, initInfo.gpInputAssemblyInfo, initInfo.gpViewportStateInfo, initInfo.gpRasterizationInfo, initInfo.gpColourBlendingInfo, initInfo.dynamicStates);
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initVertexBuffer(initInfo.verticiesBufferInfo);
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initIndexBuffer(initInfo.indexBufferData);
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
	}

	GraphicsContext::GraphicsContext(GraphicsContext&& moveFrom) : context(std::move(moveFrom.context)), swapchain(std::move(moveFrom.swapchain)), scImageViews(std::move(moveFrom.scImageViews)), graphicsPipeline(std::move(moveFrom.graphicsPipeline)), verticiesBuffer(std::move(moveFrom.verticiesBuffer)), verticiesBufferMemory(std::move(moveFrom.verticiesBufferMemory)), indicesBuffer(std::move(moveFrom.indicesBuffer)), indicesBufferMemory(std::move(moveFrom.indicesBufferMemory)), verticiesCount(std::move(moveFrom.verticiesCount)), indicesCount(std::move(moveFrom.indicesCount)), descriptorSetLayout(std::move(moveFrom.descriptorSetLayout)), uniformBuffers(std::move(moveFrom.uniformBuffers)), uniformBuffersMemory(std::move(moveFrom.uniformBuffersMemory)), uniformBuffersAddresses(std::move(moveFrom.uniformBuffersAddresses)), savedScConfigInfo(std::move(moveFrom.savedScConfigInfo)) {
	
	}

	VulkanContext& GraphicsContext::getContext() {
		return context;
	}

	void GraphicsContext::recreateSwapchain() {
		swapchain = nullptr;
		scImageViews.clear();

		initSwapchainAndImageViews(std::get<0>(savedScConfigInfo), std::get<1>(savedScConfigInfo), std::get<2>(savedScConfigInfo), std::get<3>(savedScConfigInfo), std::get<4>(savedScConfigInfo), std::get<5>(savedScConfigInfo), std::get<6>(savedScConfigInfo), std::get<7>(savedScConfigInfo), std::get<8>(savedScConfigInfo));
	}

	void GraphicsContext::initSwapchainAndImageViews(vk::SurfaceFormatKHR const& desiredFormat, uint32_t const& desiredImageCount, vk::PresentModeKHR const& desiredPresentMode, vk::ImageUsageFlags const& imageUsage, vk::ImageAspectFlags const& imageViewAspect, vk::SharingMode const& sharingMode, uint32_t const& queueFamilyAccessorCount, uint32_t* queueFamilyAccessorIndiceList, vk::SurfaceTransformFlagBitsKHR const& preTransform) {
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

	void GraphicsContext::initDescriptorSetLayout(std::vector<vk::DescriptorSetLayoutBinding> const& bindings) {
		vk::DescriptorSetLayoutCreateInfo descSetsInfo = {
			.flags = {},
			.bindingCount = static_cast<uint32_t>(bindings.size()),
			.pBindings = bindings.data()
		};

		descriptorSetLayout = vk::raii::DescriptorSetLayout(context.device, descSetsInfo);

		std::cout << "Created descriptor set layout with " << bindings.size() << " bindings\n";
	}

	void GraphicsContext::initUniformBuffers(std::tuple<uint32_t, uint32_t, vk::SharingMode> const& uboInfo) {
		uint32_t framesInFlight = std::get<0>(uboInfo);
		uint32_t uboSize = std::get<1>(uboInfo);
		
		for(int i = 0; i < framesInFlight; i++) {
			uniformBuffers.push_back(nullptr);
			uniformBuffersMemory.push_back(nullptr);
			uniformBuffersAddresses.push_back(nullptr);

			createBufferAndMemory(uniformBuffers[i], uniformBuffersMemory[i], vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, uboSize, vk::BufferUsageFlagBits::eUniformBuffer, std::get<2>(uboInfo));
			uniformBuffersAddresses[i] = uniformBuffersMemory[i].mapMemory(0, uboSize);
		}
	}

	void GraphicsContext::initGraphicsPipeline(std::vector<std::tuple<vk::ShaderStageFlagBits, const char*, const char*>> const& shaderStageInfos, std::tuple<vk::VertexInputBindingDescription, std::vector<vk::VertexInputAttributeDescription>> const& vInfo, std::tuple<vk::PrimitiveTopology, bool> const& inAssemInfo, std::tuple<std::array<float, 6>, std::array<uint32_t, 4>> const& viewInfo, std::tuple<bool, bool, vk::PolygonMode, vk::CullModeFlagBits, vk::FrontFace, bool, float, float, float, float> const& rasInfo, std::tuple<std::vector<std::tuple<bool, vk::BlendFactor, vk::BlendFactor, vk::BlendOp, vk::BlendFactor, vk::BlendFactor, vk::BlendOp, vk::ColorComponentFlags>>, std::tuple<bool, vk::LogicOp, std::array<float, 4>>> const& cBlendInfo, std::vector<vk::DynamicState> const& dyInfo) {
		std::vector<std::tuple<vk::ShaderStageFlagBits, vk::raii::ShaderModule, const char*>> shaderStageInfosConverted{};
		for(int i = 0; i < shaderStageInfos.size(); i++) {
			shaderStageInfosConverted.push_back(std::make_tuple(std::get<0>(shaderStageInfos[i]), getShaderModule(std::get<1>(shaderStageInfos[i])), std::get<2>(shaderStageInfos[i])));
		}
		std::vector<vk::PipelineShaderStageCreateInfo> shaderCreateInfo = getConfigurableShaderStageInfos(shaderStageInfosConverted);

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {
			.vertexBindingDescriptionCount = 1,
			.pVertexBindingDescriptions = &std::get<0>(vInfo),
			.vertexAttributeDescriptionCount = static_cast<uint32_t>(std::get<1>(vInfo).size()),
			.pVertexAttributeDescriptions = std::get<1>(vInfo).data()
		};

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

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo = { 
			.setLayoutCount = 1,
			.pSetLayouts = &*descriptorSetLayout,
			.pushConstantRangeCount = 0 
		};
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

		if(surfaceCapabilities.currentExtent.width == 0xFFFFFFFF) {
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

	std::vector<vk::PipelineShaderStageCreateInfo> GraphicsContext::getConfigurableShaderStageInfos(std::vector<std::tuple<vk::ShaderStageFlagBits, vk::raii::ShaderModule, const char*>> const& infos) {
		std::vector<vk::PipelineShaderStageCreateInfo> configurableShaderStageInfos{};

		for (std::tuple<vk::ShaderStageFlagBits, vk::raii::ShaderModule, const char*> const& info : infos) {
			configurableShaderStageInfos.push_back(vk::PipelineShaderStageCreateInfo {
				.stage = std::get<0>(info),
				.module = std::get<1>(info),
				.pName = std::get<2>(info)
			});
		}

		return configurableShaderStageInfos;
	}

	void GraphicsContext::initVertexBuffer(std::tuple<vk::SharingMode, std::vector<General::Vertex>> const& vbInfo) {
		verticiesCount = std::get<1>(vbInfo).size();
		uint32_t bufferSize = verticiesCount * sizeof(std::get<1>(vbInfo)[0]);
		
		vk::raii::Buffer stagingBuffer = nullptr;
		vk::raii::DeviceMemory stagingBufferMemory = nullptr;

		createBufferAndMemory(stagingBuffer, stagingBufferMemory, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, bufferSize, vk::BufferUsageFlagBits::eTransferSrc, std::get<0>(vbInfo));
		void* stagingBufferAddress = stagingBufferMemory.mapMemory(0, bufferSize);
		memcpy(stagingBufferAddress, std::get<1>(vbInfo).data(), bufferSize);
		stagingBufferMemory.unmapMemory();

		createBufferAndMemory(verticiesBuffer, verticiesBufferMemory, vk::MemoryPropertyFlagBits::eDeviceLocal, bufferSize, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, std::get<0>(vbInfo));
		copyBuffer(stagingBuffer, verticiesBuffer, 0, 0, 0, bufferSize);

		std::cout << "Created verticies buffer with size " << bufferSize << '\n';
	}

	void GraphicsContext::initIndexBuffer(std::vector<uint32_t> const& indexBufferData) {
		indicesCount = indexBufferData.size();
		uint32_t bufferSize = indicesCount * sizeof(indexBufferData[0]);

		vk::raii::Buffer stagingBuffer = nullptr;
		vk::raii::DeviceMemory stagingBufferMemory = nullptr;

		createBufferAndMemory(stagingBuffer, stagingBufferMemory, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive);
		void* stagingBufferAddress = stagingBufferMemory.mapMemory(0, bufferSize);
		memcpy(stagingBufferAddress, indexBufferData.data(), bufferSize);
		stagingBufferMemory.unmapMemory();

		createBufferAndMemory(indicesBuffer, indicesBufferMemory, vk::MemoryPropertyFlagBits::eDeviceLocal, bufferSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive);
		copyBuffer(stagingBuffer, indicesBuffer, 0, 0, 0, bufferSize);

		std::cout << "Created verticies buffer with size " << bufferSize << '\n';
	}

	void GraphicsContext::createBufferAndMemory(vk::raii::Buffer& buffer, vk::raii::DeviceMemory& memory, vk::MemoryPropertyFlags const& properties, uint32_t const& size, vk::BufferUsageFlags const& usage, vk::SharingMode const& sharingMode) {
		vk::BufferCreateInfo info = {
			.size = size,
			.usage = usage,
			.sharingMode = sharingMode
		};
		buffer = vk::raii::Buffer(context.device, info);

		vk::MemoryRequirements vbMemoryRequirements = buffer.getMemoryRequirements();
		uint32_t memoryTypeIndex = getSuitableMemoryTypeIndex(vbMemoryRequirements.memoryTypeBits, properties);
		if(memoryTypeIndex == 0xFFFFFFFF) {
			throw std::runtime_error("No suitable memory type found for buffer");
		}
		vk::MemoryAllocateInfo allocateInfo = {
			.allocationSize = vbMemoryRequirements.size,
			.memoryTypeIndex = memoryTypeIndex
		};
		memory = vk::raii::DeviceMemory(context.device, allocateInfo);

		buffer.bindMemory(memory, 0);
	}

	void GraphicsContext::copyBuffer(vk::raii::Buffer& src, vk::raii::Buffer& dst, uint32_t const& qfIndex, uint32_t const& srcOff, uint32_t const& dstOff, uint32_t const& size) {
		vk::CommandPoolCreateInfo poolInfo = {
			.flags = vk::CommandPoolCreateFlagBits::eTransient,
			.queueFamilyIndex = qfIndex
		};
		vk::raii::CommandPool tempPool = vk::raii::CommandPool(context.device, poolInfo);

		vk::CommandBufferAllocateInfo cmdBufInfo = {
			.commandPool = tempPool,
			.level = vk::CommandBufferLevel::ePrimary,
			.commandBufferCount = 1
		};
		vk::raii::CommandBuffer tempCmdBuf = std::move(vk::raii::CommandBuffers(context.device, cmdBufInfo)[0]);

		tempCmdBuf.begin(vk::CommandBufferBeginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
		tempCmdBuf.copyBuffer(src, dst, vk::BufferCopy{ .srcOffset = srcOff, .dstOffset = dstOff, .size = size});
		tempCmdBuf.end();

		context.queues[qfIndex][0].submit(vk::SubmitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*tempCmdBuf }, nullptr);
		context.device.waitIdle();
	}

	uint32_t GraphicsContext::getSuitableMemoryTypeIndex(uint32_t filter, vk::MemoryPropertyFlags const& requiredProperties) {
		vk::PhysicalDeviceMemoryProperties pdMemoryProperties = context.physicalDevice.getMemoryProperties();
		uint32_t selectedMemoryTypeIndex = 0xFFFFFFFF;

		for(int i = 0; i < pdMemoryProperties.memoryTypeCount; i++) {
			if ((filter & (1 << i)) && ((pdMemoryProperties.memoryTypes[i].propertyFlags & requiredProperties) == requiredProperties)) {
				selectedMemoryTypeIndex = i;
			}
		}

		return selectedMemoryTypeIndex;
	}

	vk::raii::ShaderModule GraphicsContext::getShaderModule(std::string const& sprivPath) {
		std::vector<char> sprivShader = fileBytes(sprivPath);

		if (sprivShader.size() == 1 && sprivShader[0] == 'x') {
			throw std::runtime_error("Failure reading spriv file at " + sprivPath);
		}

		vk::ShaderModuleCreateInfo shaderModuleInfo = {
			.codeSize = sprivShader.size() * sizeof(char),
			.pCode = reinterpret_cast<uint32_t*>(sprivShader.data())
		};
		vk::raii::ShaderModule shaderModule = context.device.createShaderModule(shaderModuleInfo);

		return shaderModule;
	}

	std::vector<char> GraphicsContext::fileBytes(std::string const& path) {
		std::ifstream fileInput(path, std::ios::ate | std::ios::binary);
		if (!fileInput.good()) {
			return { 'x' };
		}

		std::vector<char> bytes(fileInput.tellg());
		fileInput.seekg(0);
		fileInput.read(bytes.data(), bytes.size());

		return bytes;
	}
}