#include "vulkan/GraphicsEngine.h"
#include <limits>

namespace Vulkan {
	GraphicsEngine::GraphicsEngine(GraphicsContext&& context, GraphicsEngineInitInfo const& initInfo) : graphicsContext(std::move(context)), frameInFlight(0), FRAMES_IN_FLIGHT_COUNT(initInfo.framesInFlightCount), windowResized(false) {
		initCommandPool(initInfo.commandPoolsInfos);
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initCommandBuffers(initInfo.commandBuffersInfos);
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initSemaphores(initInfo.framesInFlightCount);
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initFences(initInfo.framesInFlightCount);

		glfwSetWindowUserPointer(graphicsContext.context.window, this);
		glfwSetFramebufferSizeCallback(graphicsContext.context.window, framebufferResizeCallback);
	}

	GraphicsEngine::GraphicsEngine(GraphicsEngine&& moveFrom) : graphicsContext(std::move(moveFrom.graphicsContext)), commandPools(std::move(moveFrom.commandPools)), commandBuffers(std::move(moveFrom.commandBuffers)), readyToRender(std::move(moveFrom.readyToRender)), renderingFinished(std::move(moveFrom.renderingFinished)), commandBufferFinished(std::move(moveFrom.commandBufferFinished)), frameInFlight(moveFrom.frameInFlight), FRAMES_IN_FLIGHT_COUNT(moveFrom.FRAMES_IN_FLIGHT_COUNT), windowResized(moveFrom.windowResized) {

	}

	void GraphicsEngine::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		GraphicsEngine* thisEngine = reinterpret_cast<GraphicsEngine*>(glfwGetWindowUserPointer(window));
		thisEngine->windowResized = true;
	}

	void GraphicsEngine::windowResizedAlert() {
		int width = 0, height = 0;
		glfwGetFramebufferSize(graphicsContext.context.window, &width, &height);
		while(width == 0 || height == 0) {
			glfwWaitEvents();
			glfwGetFramebufferSize(graphicsContext.context.window, &width, &height);
		}

		graphicsContext.context.device.waitIdle();
		graphicsContext.recreateSwapchain();
		recreateSemaphores();

		windowResized = false;
	}

	void GraphicsEngine::recreateSemaphores() {
		readyToRender.clear();
		renderingFinished.clear();
		initSemaphores(FRAMES_IN_FLIGHT_COUNT);
	}

	void GraphicsEngine::initCommandPool(std::vector<std::tuple<vk::CommandPoolCreateFlags, uint32_t>> const& poolInfos) {
		vk::CommandPoolCreateInfo commandPoolInfo{};

		for (std::tuple<vk::CommandPoolCreateFlags, uint32_t> const& info : poolInfos) {
			commandPoolInfo.flags = std::get<0>(info);
			commandPoolInfo.queueFamilyIndex = std::get<1>(info);

			commandPools.push_back(vk::raii::CommandPool(graphicsContext.context.device, commandPoolInfo));
		};

		std::cout << "Created " << commandPools.size() << " command pools\n";
	}

	void GraphicsEngine::initCommandBuffers(std::tuple<uint32_t, vk::CommandBufferLevel, uint32_t> const& bufInfos) {
		vk::CommandBufferAllocateInfo commandBuffersInfo = {
			.commandPool = commandPools[std::get<0>(bufInfos)],
			.level = std::get<1>(bufInfos),
			.commandBufferCount = std::get<2>(bufInfos)
		};

		commandBuffers = vk::raii::CommandBuffers(graphicsContext.context.device, commandBuffersInfo);
		
		std::cout << "Created " << std::get<2>(bufInfos) << " command buffers\n";
	}

	void GraphicsEngine::initSemaphores(uint32_t const& count) {
		for (int i = 0; i < count; i++) {
			readyToRender.push_back(vk::raii::Semaphore(graphicsContext.context.device, vk::SemaphoreCreateInfo{}));
		}

		for (int i = 0; i < count; i++) {
			renderingFinished.push_back(vk::raii::Semaphore(graphicsContext.context.device, vk::SemaphoreCreateInfo{}));
		}

		std::cout << "Created " << readyToRender.size() << " semaphores in the 2 sets\n";
	}

	void GraphicsEngine::initFences(uint32_t const& count) {
		for (int i = 0; i < count; i++) {
			commandBufferFinished.push_back(vk::raii::Fence(graphicsContext.context.device, vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled }));
		}

		std::cout << "Created " << commandBufferFinished.size() << " fences\n";
	}

	void GraphicsEngine::runLoop() {
		uint32_t nextSecondMark = 1;
		uint32_t framesInSecond = 0;

		while (!glfwWindowShouldClose(graphicsContext.context.window)) {
			glfwPollEvents();
			if (glfwGetKey(graphicsContext.context.window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
				glfwSetWindowShouldClose(graphicsContext.context.window, true);
			}
			renderAndPresentImage();

			if (glfwGetTime() <= nextSecondMark) {
				++framesInSecond;
			} else {
				std::cout << "FPS:" << framesInSecond << '\n';
				++nextSecondMark;
				framesInSecond = 0;
			}
		}

		graphicsContext.context.device.waitIdle();
	}

	// KIND OF HARD CODED NANA
	void GraphicsEngine::renderAndPresentImage() {
		while (graphicsContext.context.device.waitForFences(*commandBufferFinished[frameInFlight], true, UINT64_MAX) == vk::Result::eTimeout);

		std::pair<vk::Result, uint32_t> imageIndexPair = graphicsContext.swapchain.acquireNextImage(UINT64_MAX, readyToRender[frameInFlight], nullptr);
		if (windowResized || (imageIndexPair.first == vk::Result::eErrorOutOfDateKHR)) {
			windowResizedAlert();
			return;
		}

		graphicsContext.context.device.resetFences(*commandBufferFinished[frameInFlight]);
		
		commandBuffers[frameInFlight].reset();
		recordCommandBuffer(commandBuffers[frameInFlight], graphicsContext.swapchain.getImages()[imageIndexPair.second], graphicsContext.scImageViews[imageIndexPair.second]);

		vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		vk::SubmitInfo submitInfo = {
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &*readyToRender[frameInFlight],
			.pWaitDstStageMask = &waitStage,
			.commandBufferCount = 1,
			.pCommandBuffers = &*commandBuffers[frameInFlight],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &*renderingFinished[frameInFlight]
		};
		graphicsContext.context.queues[0][0].submit(submitInfo, *commandBufferFinished[frameInFlight]);

		vk::PresentInfoKHR presentInfo = {
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &*renderingFinished[frameInFlight],
			.swapchainCount = 1,
			.pSwapchains = &*graphicsContext.swapchain,
			.pImageIndices = &imageIndexPair.second
		};
		
		if (windowResized || (graphicsContext.context.queues[0][0].presentKHR(presentInfo) == vk::Result::eErrorOutOfDateKHR)) {
			windowResizedAlert();
			return;
		}

		frameInFlight = (frameInFlight + 1) % FRAMES_IN_FLIGHT_COUNT;
	}

	// KIND OF HARD CODED NANA
	void GraphicsEngine::recordCommandBuffer(vk::raii::CommandBuffer const& cmdBuffer, vk::Image const& image, vk::ImageView const& imageView) {
		cmdBuffer.begin({});

		transitionImageLayout(cmdBuffer, image,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::PipelineStageFlagBits2::eTopOfPipe,
			{},
			0,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::AccessFlagBits2::eColorAttachmentWrite,
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
		cmdBuffer.beginRendering(renderingInfo);
		cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsContext.graphicsPipeline);
		cmdBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(graphicsContext.getSurfaceExtent().width), static_cast<float>(graphicsContext.getSurfaceExtent().height), 0.0f, 1.0f));
		cmdBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), graphicsContext.getSurfaceExtent()));
		
		cmdBuffer.bindVertexBuffers(0, *graphicsContext.verticiesBuffer, { 0 });
		cmdBuffer.draw(graphicsContext.verticiesCount, 1, 0, 0);
		cmdBuffer.endRendering();

		transitionImageLayout(cmdBuffer, image,
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::ePresentSrcKHR,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			0,
			vk::PipelineStageFlagBits2::eBottomOfPipe,
			{},
			0,
			vk::ImageSubresourceRange {
				   .aspectMask = vk::ImageAspectFlagBits::eColor,
				   .baseMipLevel = 0,
				   .levelCount = 1,
				   .baseArrayLayer = 0,
				   .layerCount = 1 }
		);
		
		cmdBuffer.end();
	}

	// KIND OF HARD CODED NANA
	void GraphicsEngine::transitionImageLayout(vk::raii::CommandBuffer const& buffer, vk::Image const& image, vk::ImageLayout const& old, vk::ImageLayout const& newX, vk::PipelineStageFlags2 const& srcStage, vk::AccessFlags2 const& srcAccess, uint32_t const& srcQfIndex, vk::PipelineStageFlags2 const& dstStage, vk::AccessFlags2 const& dstAccess, uint32_t const& dstQfIndex, vk::ImageSubresourceRange const& range) {
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