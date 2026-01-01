#include "V_Engine.h"

namespace Vulkan {
	Engine::Engine(Context&& context, EngineInitInfo const& initInfo) : context(std::move(context)), swapchain{ nullptr }, scImageViews{}, graphicsPipeline{ nullptr }, commandPool{ nullptr }, commandBuffers{}, readyToRender{}, renderingFinished{}, commandBufferFinished{} {
	
	}
}