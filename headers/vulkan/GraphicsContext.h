#pragma once

#include "vulkan/VulkanContext.h"
#include <tuple>
#include <string>

namespace Vulkan {
	struct GraphicsContextInitInfo {
		vk::SurfaceFormatKHR swapchainFormat;
		uint32_t swapchainImageCount;
		vk::PresentModeKHR swapchainPresentMode;
		vk::ImageUsageFlagBits swapchainImageUsage;
		vk::ImageAspectFlagBits imageViewAspect;
		vk::SharingMode swapchainImageSharingMode;
		uint32_t swapchainQueueFamilyAccessorCount;
		uint32_t* swapchainQueueFamilyAccessorIndiceList;
		vk::SurfaceTransformFlagBitsKHR swapchainPreTransform;

		std::vector<std::tuple<vk::ShaderStageFlagBits, const char*, const char*>> pipelineSprivModuleInfos;
	};

	class GraphicsContext {
	private:
		VulkanContext context;
		vk::raii::SwapchainKHR swapchain;
		std::vector<vk::raii::ImageView> scImageViews;
		vk::raii::Pipeline graphicsPipeline;

		void initSwapchainAndImageViews(vk::SurfaceFormatKHR const& desiredFormat, uint32_t const& desiredImageCount, vk::PresentModeKHR const& desiredPresentMode, vk::ImageUsageFlagBits const& imageUsage, vk::ImageAspectFlagBits const& imageViewAspect, vk::SharingMode const& sharingMode, uint32_t const& queueFamilyAccessorCount, uint32_t* queueFamilyAccessorIndiceList, vk::SurfaceTransformFlagBitsKHR const& preTransform);
		void initGraphicsPipeline(std::vector<std::tuple<vk::ShaderStageFlagBits, const char*, const char*>> const& sprivInfos);

		vk::Extent2D getSurfaceExtent();
		vk::SurfaceFormatKHR getScFormat(vk::SurfaceFormatKHR const& desiredFormat);
		uint32_t getScImageCount(uint32_t const& desiredImageCount);
		vk::PresentModeKHR getScPresentMode(vk::PresentModeKHR const& desiredPresentMode);

		vk::ShaderModule getShaderModule(std::string const& sprivPath);
		std::vector<vk::PipelineShaderStageCreateInfo> getConfigurableShaderStageInfos(std::vector<std::tuple<vk::ShaderStageFlagBits, const char*, const char*>> const& infos);
		std::vector<char> fileBytes(std::string const& path);

	public:
		GraphicsContext(VulkanContext&& context, GraphicsContextInitInfo const& initInfo);
	};
}