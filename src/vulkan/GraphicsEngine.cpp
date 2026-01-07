#include "vulkan/GraphicsEngine.h"
#include <limits>

namespace Vulkan {
	GraphicsEngine::GraphicsEngine(GraphicsContext&& context, GraphicsEngineInitInfo const& initInfo) : graphicsContext(std::move(context)), FRAMES_IN_FLIGHT_COUNT(initInfo.framesInFlightCount) {
		initCommandPool(initInfo.commandPoolsInfos);
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initCommandBuffers(initInfo.commandBuffersInfos);
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initSemaphores(initInfo.framesInFlightCount);
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initFences(initInfo.framesInFlightCount);
	}

	GraphicsEngine::GraphicsEngine(GraphicsEngine&& moveFrom) : graphicsContext(std::move(moveFrom.graphicsContext)), commandPools(std::move(moveFrom.commandPools)), commandBuffers(std::move(moveFrom.commandBuffers)), readyToRender(std::move(moveFrom.readyToRender)), renderingFinished(std::move(moveFrom.renderingFinished)), commandBufferFinished(std::move(moveFrom.commandBufferFinished)), FRAMES_IN_FLIGHT_COUNT(moveFrom.FRAMES_IN_FLIGHT_COUNT) {

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

	void GraphicsEngine::initCommandBuffers(std::vector<std::tuple<uint32_t, vk::CommandBufferLevel>> const& bufInfos) {
		vk::CommandBufferAllocateInfo commandBuffersInfo{};

		for (std::tuple<uint32_t, vk::CommandBufferLevel> const& info : bufInfos) {
			commandBuffersInfo.commandPool = commandPools[std::get<0>(info)];
			commandBuffersInfo.level = std::get<1>(info);
			commandBuffersInfo.commandBufferCount = 1;

			commandBuffers.emplace_back(graphicsContext.context.device, commandBuffersInfo);
		}
		std::cout << "Created " << commandBuffers.size() << " command buffers\n";
	}

	void GraphicsEngine::initSemaphores(uint32_t const& count) {
		for (int i = 0; i < count; i++) {
			readyToRender.emplace_back(graphicsContext.context.device, vk::SemaphoreCreateInfo{});
		}

		for (int i = 0; i < count; i++) {
			renderingFinished.emplace_back(graphicsContext.context.device, vk::SemaphoreCreateInfo{});
		}

		std::cout << "Created " << readyToRender.size() << " semaphores in the 2 sets\n";
	}

	void GraphicsEngine::initFences(uint32_t const& count) {
		for (int i = 0; i < count; i++) {
			commandBufferFinished.emplace_back(graphicsContext.context.device, vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
		}

		std::cout << "Created " << commandBufferFinished.size() << " fences\n";
	}

	void GraphicsEngine::runLoop() {
		while (!glfwWindowShouldClose(graphicsContext.context.window)) {
			glfwPollEvents();
			if (glfwGetKey(graphicsContext.context.window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
				glfwSetWindowShouldClose(graphicsContext.context.window, true);
			}
			draw();
		}

		graphicsContext.context.device.waitIdle();
	}

	// KIND OF HARD CODED NANA
	void GraphicsEngine::draw() {
		while (graphicsContext.context.device.waitForFences(*commandBufferFinished[frameInFlight], true, UINT64_MAX) == vk::Result::eTimeout);
		graphicsContext.context.device.resetFences(*commandBufferFinished[frameInFlight]);

		std::pair<vk::Result, uint32_t> imageIndexPair = graphicsContext.swapchain.acquireNextImage(UINT64_MAX, readyToRender[frameInFlight], nullptr);
		commandBuffers[0].clear();
		recordCommandBuffer(commandBuffers[0], graphicsContext.swapchain.getImages()[imageIndexPair.second], graphicsContext.scImageViews[imageIndexPair.second]);

		vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		vk::SubmitInfo submitInfo = {
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &*readyToRender[frameInFlight],
			.pWaitDstStageMask = &waitStage,
			.commandBufferCount = 1,
			.pCommandBuffers = &*commandBuffers[0],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &*renderingFinished[frameInFlight]
		};
		graphicsContext.context.queues[0][0].submit(submitInfo, commandBufferFinished[frameInFlight]);

		vk::PresentInfoKHR presentInfo = {
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &*renderingFinished[frameInFlight],
			.swapchainCount = 1,
			.pSwapchains = &*graphicsContext.swapchain,
			.pImageIndices = &imageIndexPair.second
		};
		graphicsContext.context.queues[0][0].presentKHR(presentInfo);

		frameInFlight = (frameInFlight + 1) % FRAMES_IN_FLIGHT_COUNT;
	}

	// KIND OF HARD CODED NANA
	void GraphicsEngine::recordCommandBuffer(vk::raii::CommandBuffer const& buffer, vk::Image const& image, vk::ImageView const& imageView) {
		buffer.begin({});

		transitionImageLayout(buffer, image,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::PipelineStageFlagBits2::eTopOfPipe,
			{},
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			0,
			0,
			vk::ImageSubresourceRange {
				   .aspectMask = vk::ImageAspectFlagBits::eColor,
				   .baseMipLevel = 0,
				   .levelCount = 1,
				   .baseArrayLayer = 0,
				   .layerCount = 1 }
		);

		vk::RenderingAttachmentInfo attachmentInfo = {
			.imageView = imageView,
			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eStore,
			.clearValue = vk::ClearColorValue(0.3f, 0.3f, 0.3f, 1.0f)
		};
		vk::RenderingInfo renderingInfo = {
			.renderArea = vk::Rect2D{ .offset = {0, 0}, .extent = graphicsContext.getSurfaceExtent() },
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &attachmentInfo 
		};
		buffer.beginRendering(renderingInfo);
		buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsContext.graphicsPipeline);
		buffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(graphicsContext.getSurfaceExtent().width), static_cast<float>(graphicsContext.getSurfaceExtent().height), 0.0f, 1.0f));
		buffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), graphicsContext.getSurfaceExtent()));
		buffer.draw(3, 1, 0, 0);
		buffer.endRendering();

		transitionImageLayout(buffer, image,
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::ePresentSrcKHR,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::PipelineStageFlagBits2::eBottomOfPipe,
			{},
			0,
			0,
			vk::ImageSubresourceRange{}
		);
		
		buffer.end();
	}

	// KIND OF HARD CODED NANA
	void GraphicsEngine::transitionImageLayout(vk::raii::CommandBuffer const& buffer, vk::Image const& image, vk::ImageLayout const& old, vk::ImageLayout const& newX, vk::PipelineStageFlags2 const& srcStage, vk::AccessFlags2 const& srcAccess, vk::PipelineStageFlags2 const& dstStage, vk::AccessFlags2 const& dstAccess, uint32_t const& srcQfIndex, uint32_t const& dstQfIndex, vk::ImageSubresourceRange const& range) {
		vk::ImageMemoryBarrier2 imageTransitionBarrier = {
			.srcStageMask = srcStage,
			.srcAccessMask = srcAccess,
			.dstStageMask = dstStage,
			.dstAccessMask = dstAccess,
			.oldLayout = old,
			.newLayout = newX,
			.srcQueueFamilyIndex = srcQfIndex,
			.dstQueueFamilyIndex = dstQfIndex,
			.image = image,
			.subresourceRange = range
		};

		vk::DependencyInfo dependencyInfo = {
			.dependencyFlags = {},
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &imageTransitionBarrier 
		};

		buffer.pipelineBarrier2(dependencyInfo);
	}
}