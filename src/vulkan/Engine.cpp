#include "vulkan/Engine.h"

namespace Vulkan {
	Engine::Engine(Context&& context, EngineInitInfo const& initInfo) : context(std::move(context)), swapchain{ nullptr }, scImageViews{}, graphicsPipeline{ nullptr }, commandPool{ nullptr }, commandBuffers{}, readyToRender{}, renderingFinished{}, commandBufferFinished{} {
		initSwapchainAndImageViews(initInfo.swapchainFormat, initInfo.swapchainImageCount, initInfo.swapchainPresentMode, initInfo.swapchainImageUsage, initInfo.imageViewAspect, initInfo.swapchainImageSharingMode, initInfo.swapchainQueueFamilyAccessorCount, initInfo.swapchainQueueFamilyAccessorIndiceList, initInfo.swapchainPreTransform);
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initGraphicsPipeline();
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initCommandPool();
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initCommandBuffers();
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initSemaphores();
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initFences();
	}

	void Engine::initSwapchainAndImageViews(vk::SurfaceFormatKHR const& desiredFormat, uint32_t const& desiredImageCount, vk::PresentModeKHR const& desiredPresentMode, vk::ImageUsageFlagBits const& imageUsage, vk::ImageAspectFlagBits const& imageViewAspect, vk::SharingMode const& sharingMode, uint32_t const& queueFamilyAccessorCount, uint32_t* queueFamilyAccessorIndiceList, vk::SurfaceTransformFlagBitsKHR const& preTransform) {
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

	void Engine::initGraphicsPipeline() {
		
	}

	void Engine::initCommandPool() {
		
	}

	void Engine::initCommandBuffers() {
		
	}

	void Engine::initSemaphores() {
		
	}

	void Engine::initFences() {
		
	}

	// already decided by how large the window is
	vk::Extent2D Engine::getSurfaceExtent() {
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
	vk::SurfaceFormatKHR Engine::getScFormat(vk::SurfaceFormatKHR const& desiredFormat) {
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
	uint32_t Engine::getScImageCount(uint32_t const& desiredImageCount) {
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
	vk::PresentModeKHR Engine::getScPresentMode(vk::PresentModeKHR const& desiredPresentMode) {
		std::vector<vk::PresentModeKHR> compatiblePresentModes = context.physicalDevice.getSurfacePresentModesKHR(context.surface);
		vk::PresentModeKHR selectedPresentMode{};

		for(vk::PresentModeKHR const& compPreMode : compatiblePresentModes) {
			if(compPreMode == desiredPresentMode) {
				selectedPresentMode = compPreMode;
			}
		}

		return selectedPresentMode;
	}
}