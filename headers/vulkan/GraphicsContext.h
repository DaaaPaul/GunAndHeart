#pragma once

#include "vulkan/VulkanContext.h"
#include <tuple>
#include <string>

namespace Vulkan {
	class GraphicsEngine;

	struct GraphicsContextInitInfo {
		vk::SurfaceFormatKHR scFormat;
		uint32_t scImageCount;
		vk::PresentModeKHR scPresentMode;
		vk::ImageUsageFlagBits scImageUsage;
		vk::ImageAspectFlagBits scImageViewAspect;
		vk::SharingMode scImageSharingMode;
		uint32_t scQueueFamilyAccessorCount;
		uint32_t* scQueueFamilyAccessorIndiceList;
		vk::SurfaceTransformFlagBitsKHR scPreTransform;

		std::vector<std::tuple<vk::ShaderStageFlagBits, const char*, const char*>> const& gpShaderStageInfos;
		// VERTEX INPUT INFO MISSING NANA
		std::tuple<vk::PrimitiveTopology, bool> gpInputAssemblyInfo;
		std::tuple<std::array<float, 6>, std::array<uint32_t, 4>> gpViewportStateInfo;
		std::tuple<bool, bool, vk::PolygonMode, vk::CullModeFlagBits, vk::FrontFace, bool, float, float, float, float> gpRasterizationInfo;
		// MULTISAMPLING INFO MISSING NANA
		std::tuple<
			std::vector<std::tuple<bool, vk::BlendFactor, vk::BlendFactor, vk::BlendOp, vk::BlendFactor, vk::BlendFactor, vk::BlendOp, vk::ColorComponentFlags>>,
			std::tuple<bool, vk::LogicOp, std::array<float, 4>>> gpColourBlendingInfo;
		std::vector<vk::DynamicState> dynamicStates;
		// LAYOUT INFO MISSING NANA
	};

	class GraphicsContext {
	private:
		VulkanContext context;
		vk::raii::SwapchainKHR swapchain;
		std::vector<vk::raii::ImageView> scImageViews;
		vk::raii::Pipeline graphicsPipeline;

		void initSwapchainAndImageViews(vk::SurfaceFormatKHR const& desiredFormat, uint32_t const& desiredImageCount, vk::PresentModeKHR const& desiredPresentMode, vk::ImageUsageFlagBits const& imageUsage, vk::ImageAspectFlagBits const& imageViewAspect, vk::SharingMode const& sharingMode, uint32_t const& queueFamilyAccessorCount, uint32_t* queueFamilyAccessorIndiceList, vk::SurfaceTransformFlagBitsKHR const& preTransform);
		void initGraphicsPipeline(std::vector<std::tuple<vk::ShaderStageFlagBits, const char*, const char*>> const& shaderStageInfos, std::tuple<vk::PrimitiveTopology, bool> const& inAssemInfo, std::tuple<std::array<float, 6>, std::array<uint32_t, 4>> const& viewInfo, std::tuple<bool, bool, vk::PolygonMode, vk::CullModeFlagBits, vk::FrontFace, bool, float, float, float, float> const& rasInfo, std::tuple<std::vector<std::tuple<bool, vk::BlendFactor, vk::BlendFactor, vk::BlendOp, vk::BlendFactor, vk::BlendFactor, vk::BlendOp, vk::ColorComponentFlags>>, std::tuple<bool, vk::LogicOp, std::array<float, 4>>> const& cBlendInfo, std::vector<vk::DynamicState> const& dyInfo);

		vk::Extent2D getSurfaceExtent();
		vk::SurfaceFormatKHR getScFormat(vk::SurfaceFormatKHR const& desiredFormat);
		uint32_t getScImageCount(uint32_t const& desiredImageCount);
		vk::PresentModeKHR getScPresentMode(vk::PresentModeKHR const& desiredPresentMode);

		std::vector<vk::PipelineShaderStageCreateInfo> getConfigurableShaderStageInfos(std::vector<std::tuple<vk::ShaderStageFlagBits, vk::raii::ShaderModule, const char*>> const& infos);
		vk::raii::ShaderModule getShaderModule(std::string const& sprivPath);
		std::vector<char> fileBytes(std::string const& path);

	public:
		friend class GraphicsEngine;

		GraphicsContext(VulkanContext&& context, GraphicsContextInitInfo const& initInfo);
		GraphicsContext(GraphicsContext&& moveFrom);
	};
}