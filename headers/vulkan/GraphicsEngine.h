#pragma once

#pragma once

#include "vulkan/GraphicsContext.h"
#include <tuple>
#include <string>
#include <utility>

namespace Vulkan {
	struct GraphicsEngineInitInfo {
		std::vector<std::tuple<vk::CommandPoolCreateFlags, uint32_t>> commandPoolInfos;
		std::vector<std::tuple<uint32_t, vk::CommandBufferLevel, uint32_t>> commandBufferInfos;
	};

	class GraphicsEngine {
	private:
		GraphicsContext graphicsContext;
		std::vector<vk::raii::CommandPool> commandPools;
		std::vector<vk::raii::CommandBuffers> commandBuffers;
		std::vector<vk::raii::Semaphore> readyToRender;
		std::vector<vk::raii::Semaphore> renderingFinished;
		std::vector<vk::raii::Fence> commandBufferFinished;

		void initCommandPool(std::vector<std::tuple<vk::CommandPoolCreateFlags, uint32_t>> const& poolInfos);
		void initCommandBuffers(std::vector<std::tuple<uint32_t, vk::CommandBufferLevel, uint32_t>> const& bufInfos);
		void initSemaphores();
		void initFences();
	public:
		GraphicsEngine(GraphicsContext&& context, GraphicsEngineInitInfo const& initInfo);
		GraphicsEngine(GraphicsEngine&& moveFrom);

		GraphicsEngine(GraphicsEngine const& copyFrom) = delete;
		GraphicsEngine& operator=(GraphicsEngine const& assignFrom) = delete;
	};
}