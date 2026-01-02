#pragma once

#include "V_Context.h"

namespace Vulkan {
	struct EngineInitInfo {
		vk::SurfaceFormatKHR swapchainFormat;
		uint32_t swapchainImageCount;
		vk::PresentModeKHR swapchainPresentMode;
		vk::ImageUsageFlagBits swapchainImageAttachment;
		vk::SharingMode swapchainImageSharingMode;
		uint32_t swapchainQueueFamilyAccessorCount;
		uint32_t* swapchainQueueFamilyAccessorIndiceList;
		vk::SurfaceTransformFlagBitsKHR swapchainPreTransform;
	};

	class Engine {
	private:
		Context context;
		vk::raii::SwapchainKHR swapchain;
		std::vector<vk::raii::ImageView> scImageViews;
		vk::raii::Pipeline graphicsPipeline;
		vk::raii::CommandPool commandPool;
		std::vector<vk::raii::CommandBuffer> commandBuffers;
		std::vector<vk::raii::Semaphore> readyToRender;
		std::vector<vk::raii::Semaphore> renderingFinished;
		std::vector<vk::raii::Fence> commandBufferFinished;

		void initSwapchain(vk::SurfaceFormatKHR const& desiredFormat, uint32_t const& desiredImageCount, vk::PresentModeKHR const& desiredPresentMode, vk::ImageUsageFlagBits const& attachment, vk::SharingMode const& sharingMode, uint32_t const& queueFamilyAccessorCount, uint32_t* queueFamilyAccessorIndiceList, vk::SurfaceTransformFlagBitsKHR const& preTransform);
		void initGraphicsPipeline();
		void initCommandPool();
		void initCommandBuffers();
		void initSemaphores();
		void initFences();

		vk::Extent2D getSurfaceExtent();
		vk::SurfaceFormatKHR getScFormat(vk::SurfaceFormatKHR const& desiredFormat);
		uint32_t getScImageCount(uint32_t const& desiredImageCount);
		vk::PresentModeKHR getScPresentMode(vk::PresentModeKHR const& desiredPresentMode);

	public:
		Engine(Context&& context, EngineInitInfo const& initInfo);
	};
}