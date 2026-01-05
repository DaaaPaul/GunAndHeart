#include "vulkan/GraphicsEngine.h"

namespace Vulkan {
	GraphicsEngine::GraphicsEngine(GraphicsContext&& context, GraphicsEngineInitInfo const& initInfo) : graphicsContext(std::move(context)) {
		initCommandPool();
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initCommandBuffers();
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initSemaphores();
		std::cout << "-------------------------------------------------------------------------------------------------------\n";
		initFences();
	}

	void initCommandPool() {
	
	}

	void initCommandBuffers() {
	
	}

	void initSemaphores() {
	
	}

	void initFences() {
	
	}
}