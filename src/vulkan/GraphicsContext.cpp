#include "vulkan/GraphicsContext.h"
#include <fstream>

namespace Vulkan {
	GraphicsContext::GraphicsContext(VulkanContext&& context, GraphicsContextInitInfo const& initInfo) : context(std::move(context)), swapchain{ nullptr }, scImageViews{}, graphicsPipeline{ nullptr } {
		initSwapchainAndImageViews(initInfo.swapchainFormat, initInfo.swapchainImageCount, initInfo.swapchainPresentMode, initInfo.swapchainImageUsage, initInfo.imageViewAspect, initInfo.swapchainImageSharingMode, initInfo.swapchainQueueFamilyAccessorCount, initInfo.swapchainQueueFamilyAccessorIndiceList, initInfo.swapchainPreTransform);
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initGraphicsPipeline(initInfo.pipelineSprivModulePath);
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

	void GraphicsContext::initGraphicsPipeline(std::string const& sprivPath) {
		////std::vector<std::tuple<vk::ShaderStageFlagBits, vk::raii::ShaderModule, const char*>> configurableStageInfos = {
		////	{vk::ShaderStageFlagBits::eVertex, getShaderModule(sprivPath), "vertexShader"},
		////	{vk::ShaderStageFlagBits::eFragment, getShaderModule(sprivPath), "fragmentShader"}
		////};
		//std::vector<vk::PipelineShaderStageCreateInfo> vertexAndFragmentShader = getConfigurableShaderStageInfos(configurableStageInfos);

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

	vk::raii::ShaderModule GraphicsContext::getShaderModule(std::string const& sprivPath) {
		std::vector<char> sprivShader = fileBytes(sprivPath);

		vk::ShaderModuleCreateInfo shaderModuleInfo = {
			.codeSize = sprivShader.size() * sizeof(char),
			.pCode = reinterpret_cast<uint32_t*>(sprivShader.data())
		};
		vk::raii::ShaderModule shaderModule(context.device, shaderModuleInfo);

		return shaderModule;
	}

	std::vector<vk::PipelineShaderStageCreateInfo> GraphicsContext::getConfigurableShaderStageInfos(std::vector<std::tuple<vk::ShaderStageFlagBits, vk::raii::ShaderModule, const char*>> const& infos) {
		std::vector<vk::PipelineShaderStageCreateInfo> configurableShaderStageInfos{};

		for(std::tuple<vk::ShaderStageFlagBits, vk::raii::ShaderModule, const char*> const& info : infos) {
			configurableShaderStageInfos.push_back(vk::PipelineShaderStageCreateInfo{
				.stage = std::get<0>(info),
				.module = std::get<1>(info),
				.pName = std::get<2>(info)
			});
		}

		return configurableShaderStageInfos;
	}

	std::vector<char> GraphicsContext::fileBytes(std::string const& path) {
		std::ifstream fileInput(path, std::ios::ate | std::ios::binary);
		if(!fileInput.good()) {
			return { 'x' };
		}

		std::vector<char> bytes(fileInput.tellg());
		fileInput.seekg(0);
		fileInput.read(bytes.data(), bytes.size());

		return bytes;
	}
}