#include "V_Context.h"

namespace Vulkan {
	Context::Context() : 
		window{ nullptr }, 
		context{}, 
		instance{ nullptr }, 
		surface{ nullptr }, 
		physicalDevice{ nullptr }, 
		device{ nullptr }, 
		graphicsQueue{ nullptr } { }

	Context::~Context() {
		
	}

	Context& Context::get() {
		static Context context;
		return context;
	}

	void Context::init() {
		initWindow();
		initInstance();
		initSurface();
		initPhysicalDevice();
		initDevice();
	}

	void Context::initWindow() {
	
	}

	void Context::initInstance() {
	
	}

	void Context::initSurface() {
	
	}

	void Context::initPhysicalDevice() {
	
	}

	void Context::initDevice() {
	
	}
}