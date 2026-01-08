#include "general/Vertex.h"

namespace General {
	vk::VertexInputBindingDescription Vertex::getVertexInputBindingDescription() {
		vk::VertexInputBindingDescription bindingInfo = {
			.binding = 0,
			.stride = sizeof(Vertex),
			.inputRate = vk::VertexInputRate::eVertex
		};

		return bindingInfo;
	}

	std::vector<vk::VertexInputAttributeDescription> Vertex::getVertexInputAttributeDescription() {
		std::vector<vk::VertexInputAttributeDescription> attributeInfos = {
			vk::VertexInputAttributeDescription{
				.location = 0,
				.binding = 0,
				.format = vk::Format::eR32G32B32Sfloat,
				.offset = offsetof(Vertex, colour)
			},
			vk::VertexInputAttributeDescription{
				.location = 1,
				.binding = 0,
				.format = vk::Format::eR32G32Sfloat,
				.offset = offsetof(Vertex, position)
			}
		};

		return attributeInfos;
	}
}