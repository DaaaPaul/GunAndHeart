#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan_raii.hpp"
#include <glm/glm.hpp>

namespace General {
	struct VertexTransformations {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 projection;

		static vk::DescriptorSetLayoutBinding getDescriptorSetLayoutBinding(uint32_t const& bindingNum, uint32_t const& descCount);
	};
}