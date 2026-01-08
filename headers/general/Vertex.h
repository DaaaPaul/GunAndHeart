#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "vulkan/vulkan_raii.hpp"
#include <glm/glm.hpp>
#include <array>

namespace General {
	struct Vertex {
		glm::vec3 colour;
		glm::vec2 position;

		static vk::VertexInputBindingDescription getVertexInputBindingDescription();
		static std::vector<vk::VertexInputAttributeDescription> getVertexInputAttributeDescription();
	};
}