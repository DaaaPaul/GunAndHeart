#pragma once

#include "vulkan/VulkanContext.h"
#include "general/Vertex.h"
#include "general/VertexTransformations.h"
#include <tuple>
#include <string>

namespace Vulkan {
	class GraphicsEngine;

	struct GraphicsContextInitInfo {
		vk::SurfaceFormatKHR scFormat;
		uint32_t scImageCount;
		vk::PresentModeKHR scPresentMode;
		vk::ImageUsageFlags scImageUsage;
		vk::ImageAspectFlags scImageViewAspect;
		vk::SharingMode scImageSharingMode;
		uint32_t scQueueFamilyAccessorCount;
		uint32_t* scQueueFamilyAccessorIndiceList;
		vk::SurfaceTransformFlagBitsKHR scPreTransform;

		std::vector<vk::DescriptorSetLayoutBinding> descriptorSetLayoutBindings;
		std::tuple<uint32_t, uint32_t, vk::SharingMode> uniformBufferInfo;

		std::vector<std::tuple<vk::ShaderStageFlagBits, const char*, const char*>> const& gpShaderStageInfos;
		std::tuple<vk::VertexInputBindingDescription, std::vector<vk::VertexInputAttributeDescription>> gpVertexInputInfo;
		std::tuple<vk::PrimitiveTopology, bool> gpInputAssemblyInfo;
		std::tuple<std::array<float, 6>, std::array<uint32_t, 4>> gpViewportStateInfo;
		std::tuple<bool, bool, vk::PolygonMode, vk::CullModeFlagBits, vk::FrontFace, bool, float, float, float, float> gpRasterizationInfo;
		// MULTISAMPLING INFO MISSING NANA
		std::tuple<
			std::vector<std::tuple<bool, vk::BlendFactor, vk::BlendFactor, vk::BlendOp, vk::BlendFactor, vk::BlendFactor, vk::BlendOp, vk::ColorComponentFlags>>,
			std::tuple<bool, vk::LogicOp, std::array<float, 4>>> gpColourBlendingInfo;
		std::vector<vk::DynamicState> dynamicStates;

		std::tuple<vk::SharingMode, std::vector<General::Vertex>> verticiesBufferInfo;
		std::vector<uint32_t> indexBufferData;
	};

	class GraphicsContext {
	private:
		VulkanContext context;
		vk::raii::SwapchainKHR swapchain;
		std::vector<vk::raii::ImageView> scImageViews;
		vk::raii::Pipeline graphicsPipeline;

		vk::raii::Buffer verticiesBuffer;
		vk::raii::DeviceMemory verticiesBufferMemory;
		vk::raii::Buffer indicesBuffer;
		vk::raii::DeviceMemory indicesBufferMemory;
		uint32_t verticiesCount;
		uint32_t indicesCount;

		vk::raii::DescriptorSetLayout descriptorSetLayout;
		// KIND OF HARD CODED NANA
		std::vector<vk::raii::Buffer> uniformBuffers;
		std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
		std::vector<void*> uniformBuffersAddresses;

		std::tuple<vk::SurfaceFormatKHR, uint32_t, vk::PresentModeKHR, vk::ImageUsageFlags, vk::ImageAspectFlags, vk::SharingMode, uint32_t, uint32_t*, vk::SurfaceTransformFlagBitsKHR> savedScConfigInfo;
		void recreateSwapchain();

		void initSwapchainAndImageViews(vk::SurfaceFormatKHR const& desiredFormat, uint32_t const& desiredImageCount, vk::PresentModeKHR const& desiredPresentMode, vk::ImageUsageFlags const& imageUsage, vk::ImageAspectFlags const& imageViewAspect, vk::SharingMode const& sharingMode, uint32_t const& queueFamilyAccessorCount, uint32_t* queueFamilyAccessorIndiceList, vk::SurfaceTransformFlagBitsKHR const& preTransform);
		void initDescriptorSetLayout(std::vector<vk::DescriptorSetLayoutBinding> const& bindings);
		void initUniformBuffers(std::tuple<uint32_t, uint32_t, vk::SharingMode> const& uboInfo);
		void initGraphicsPipeline(std::vector<std::tuple<vk::ShaderStageFlagBits, const char*, const char*>> const& shaderStageInfos, std::tuple<vk::VertexInputBindingDescription, std::vector<vk::VertexInputAttributeDescription>> const& vInfo, std::tuple<vk::PrimitiveTopology, bool> const& inAssemInfo, std::tuple<std::array<float, 6>, std::array<uint32_t, 4>> const& viewInfo, std::tuple<bool, bool, vk::PolygonMode, vk::CullModeFlagBits, vk::FrontFace, bool, float, float, float, float> const& rasInfo, std::tuple<std::vector<std::tuple<bool, vk::BlendFactor, vk::BlendFactor, vk::BlendOp, vk::BlendFactor, vk::BlendFactor, vk::BlendOp, vk::ColorComponentFlags>>, std::tuple<bool, vk::LogicOp, std::array<float, 4>>> const& cBlendInfo, std::vector<vk::DynamicState> const& dyInfo);
		void initVertexBuffer(std::tuple<vk::SharingMode, std::vector<General::Vertex>> const& vbInfo);
		void initIndexBuffer(std::vector<uint32_t> const& indexBufferData);

		vk::Extent2D getSurfaceExtent();
		vk::SurfaceFormatKHR getScFormat(vk::SurfaceFormatKHR const& desiredFormat);
		uint32_t getScImageCount(uint32_t const& desiredImageCount);
		vk::PresentModeKHR getScPresentMode(vk::PresentModeKHR const& desiredPresentMode);

		std::vector<vk::PipelineShaderStageCreateInfo> getConfigurableShaderStageInfos(std::vector<std::tuple<vk::ShaderStageFlagBits, vk::raii::ShaderModule, const char*>> const& infos);
		vk::raii::ShaderModule getShaderModule(std::string const& sprivPath);
		std::vector<char> fileBytes(std::string const& path);

		void createBufferAndMemory(vk::raii::Buffer& buffer, vk::raii::DeviceMemory& memory, vk::MemoryPropertyFlags const& properties, uint32_t const& size, vk::BufferUsageFlags const& usage, vk::SharingMode const& sharingMode);
		void copyBuffer(vk::raii::Buffer& src, vk::raii::Buffer& dst, uint32_t const& qfIndex, uint32_t const& srcOff, uint32_t const& dstOff, uint32_t const& size);
		uint32_t getSuitableMemoryTypeIndex(uint32_t filter, vk::MemoryPropertyFlags const& requiredProperties);
	public:
		friend class GraphicsEngine;

		GraphicsContext(VulkanContext&& context, GraphicsContextInitInfo const& initInfo);
		GraphicsContext(GraphicsContext&& moveFrom);
		
		GraphicsContext(GraphicsContext const& copyFrom) = delete;
		GraphicsContext& operator=(GraphicsContext const& assignFrom) = delete;
		
		VulkanContext& getContext();
	};
}