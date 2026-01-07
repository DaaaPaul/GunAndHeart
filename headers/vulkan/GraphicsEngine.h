#pragma once

#pragma once

#include "vulkan/GraphicsContext.h"
#include <tuple>
#include <string>
#include <utility>

namespace Vulkan {
	struct GraphicsEngineInitInfo {
		std::vector<std::tuple<vk::CommandPoolCreateFlags, uint32_t>> commandPoolsInfos;
		std::vector<std::tuple<uint32_t, vk::CommandBufferLevel, uint32_t>> commandBuffersInfos;
		std::tuple<uint32_t, uint32_t> semaphoresInfos;
		std::tuple<uint32_t, vk::FenceCreateFlags> fencesInfos;
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
		void initSemaphores(std::tuple<uint32_t, uint32_t> const& semInfos);
		void initFences(std::tuple<uint32_t, vk::FenceCreateFlags> const& fenInfos);
	public:
		GraphicsEngine(GraphicsContext&& context, GraphicsEngineInitInfo const& initInfo);
		GraphicsEngine(GraphicsEngine&& moveFrom);

		GraphicsEngine(GraphicsEngine const& copyFrom) = delete;
		GraphicsEngine& operator=(GraphicsEngine const& assignFrom) = delete;
	};
}