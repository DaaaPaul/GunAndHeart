#include <iostream>
#include "V_Context.h"
#include "V_Engine.h"

int main() {
	try {
		Vulkan::Context& context = Vulkan::Context::get();
		Vulkan::ContextInitInfo contextInfo = {
			.windowWidth = 800,
			.windowHeight = 800,
			.appName = "GunAndHeart",
			.validationLayers = {"VK_LAYER_KHRONOS_validation"},
		};

		context.init(contextInfo);
	} catch(std::exception const& e) {
		std::cout << e.what() << '\n';
	}
}