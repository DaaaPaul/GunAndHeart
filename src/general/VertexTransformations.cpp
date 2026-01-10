#include "general/VertexTransformations.h"

namespace General {
	vk::DescriptorSetLayoutBinding VertexTransformations::getDescriptorSetLayoutBinding(uint32_t const& bindingNum, uint32_t const& descCount) {
		vk::DescriptorSetLayoutBinding descSetBinding = {
			.binding = bindingNum,
			.descriptorType = vk::DescriptorType::eUniformBuffer,
			.descriptorCount = descCount,
			.stageFlags = vk::ShaderStageFlagBits::eVertex,
			.pImmutableSamplers = nullptr
		};

		return descSetBinding;
	}
}