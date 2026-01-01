#pragma once

#include "V_Context.h"

namespace Vulkan {
	struct EngineInitInfo {
		
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

		void initSwapchain();
		void initGraphicsPipeline();
		void initCommandPool();
		void initCommandBuffers();
		void initSemaphores();
		void initFences();

	public:
		Engine(Context&& context, EngineInitInfo const& initInfo);
	};
}