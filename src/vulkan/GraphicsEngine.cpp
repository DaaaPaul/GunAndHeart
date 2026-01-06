#include "vulkan/GraphicsEngine.h"

namespace Vulkan {
	GraphicsEngine::GraphicsEngine(GraphicsContext&& context, GraphicsEngineInitInfo const& initInfo) : graphicsContext(std::move(context)) {
		initCommandPool(initInfo.commandPoolInfos);
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initCommandBuffers(initInfo.commandBufferInfos);
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initSemaphores();
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initFences();
	}

	GraphicsEngine::GraphicsEngine(GraphicsEngine&& moveFrom) : graphicsContext(std::move(moveFrom.graphicsContext)), commandPools(std::move(moveFrom.commandPools)), commandBuffers(std::move(moveFrom.commandBuffers)), readyToRender(std::move(moveFrom.readyToRender)), renderingFinished(std::move(moveFrom.renderingFinished)), commandBufferFinished(std::move(moveFrom.commandBufferFinished)) {
	
	}

	void GraphicsEngine::initCommandPool(std::vector<std::tuple<vk::CommandPoolCreateFlags, uint32_t>> const& poolInfos) {
		vk::CommandPoolCreateInfo commandPoolInfo{};

		for (std::tuple<vk::CommandPoolCreateFlags, uint32_t> const& info : poolInfos) {
			commandPoolInfo.flags = std::get<0>(info);
			commandPoolInfo.queueFamilyIndex = std::get<1>(info);
			
			commandPools.emplace_back(graphicsContext.context.device, commandPoolInfo);
		};

		std::cout << "Created " << commandPools.size() << " command pools\n";
	}

	void GraphicsEngine::initCommandBuffers(std::vector<std::tuple<uint32_t, vk::CommandBufferLevel, uint32_t>> const& bufInfos) {
		vk::CommandBufferAllocateInfo commandBuffersInfo{};

		for(std::tuple<uint32_t, vk::CommandBufferLevel, uint32_t> const& info : bufInfos) {
			commandBuffersInfo.commandPool = commandPools[std::get<0>(info)];
			commandBuffersInfo.level = std::get<1>(info);
			commandBuffersInfo.commandBufferCount = std::get<2>(info);

			commandBuffers.emplace_back(graphicsContext.context.device, commandBuffersInfo);

			std::cout << "Created " << commandBuffersInfo.commandBufferCount << " command buffers for command pool " << std::get<0>(info) << '\n';
		}
	}

	void GraphicsEngine::initSemaphores() {
	
	}

	void GraphicsEngine::initFences() {
	
	}
}