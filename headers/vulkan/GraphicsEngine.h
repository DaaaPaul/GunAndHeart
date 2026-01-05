#pragma once

#pragma once

#include "vulkan/GraphicsContext.h"
#include <tuple>
#include <string>
#include <utility>

namespace Vulkan {
	struct GraphicsEngineInitInfo {

	};

	class GraphicsEngine {
	private:
		GraphicsContext graphicsContext;
		vk::raii::CommandPool commandPool;
		std::vector<vk::raii::CommandBuffer> commandBuffers;
		std::vector<vk::raii::Semaphore> readyToRender;
		std::vector<vk::raii::Semaphore> renderingFinished;
		std::vector<vk::raii::Fence> commandBufferFinished;

		void initCommandPool();
		void initCommandBuffers();
		void initSemaphores();
		void initFences();
	public:
		GraphicsEngine(GraphicsContext&& context, GraphicsEngineInitInfo const& initInfo);
	};
}