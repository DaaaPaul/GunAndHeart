#pragma once

#pragma once

#include "vulkan/GraphicsContext.h"
#include <tuple>
#include <string>
#include <utility>

namespace Vulkan {
	struct GraphicsEngineInitInfo {
		std::vector<std::tuple<vk::CommandPoolCreateFlags, uint32_t>> commandPoolsInfos;
		std::vector<std::tuple<uint32_t, vk::CommandBufferLevel>> commandBuffersInfos;
		uint32_t framesInFlightCount;
	};

	class GraphicsEngine {
	private:
		GraphicsContext graphicsContext;
		std::vector<vk::raii::CommandPool> commandPools;
		std::vector<vk::raii::CommandBuffer> commandBuffers;
		std::vector<vk::raii::Semaphore> readyToRender;
		std::vector<vk::raii::Semaphore> renderingFinished;
		std::vector<vk::raii::Fence> commandBufferFinished;

		uint32_t frameInFlight;
		const uint32_t FRAMES_IN_FLIGHT_COUNT;

		void initCommandPool(std::vector<std::tuple<vk::CommandPoolCreateFlags, uint32_t>> const& poolInfos);
		void initCommandBuffers(std::vector<std::tuple<uint32_t, vk::CommandBufferLevel>> const& bufInfos);
		void initSemaphores(uint32_t const& count);
		void initFences(uint32_t const& count);

		void draw();
		void recordCommandBuffer(vk::raii::CommandBuffer const& buffer, vk::Image const& image, vk::ImageView const& imageView);
		void transitionImageLayout(vk::raii::CommandBuffer const& buffer, vk::Image const& image, vk::ImageLayout const& old, vk::ImageLayout const& newX, vk::PipelineStageFlags2 const& srcStage, vk::AccessFlags2 const& srcAccess, vk::PipelineStageFlags2 const& dstStage, vk::AccessFlags2 const& dstAccess, uint32_t const& srcQfIndex, uint32_t const& dstQfIndex, vk::ImageSubresourceRange const& range);
	public:
		void runLoop();

		GraphicsEngine(GraphicsContext&& context, GraphicsEngineInitInfo const& initInfo);
		GraphicsEngine(GraphicsEngine&& moveFrom);

		GraphicsEngine(GraphicsEngine const& copyFrom) = delete;
		GraphicsEngine& operator=(GraphicsEngine const& assignFrom) = delete;
	};
}