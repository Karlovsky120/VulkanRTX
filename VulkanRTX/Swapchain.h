#pragma once

#include "GlobalDefines.h"

#include "VulkanInclude.h"

#include <vector>

class Swapchain {
public:
	vk::UniqueSwapchainKHR m_swapchain;
	std::vector<vk::UniqueFramebuffer> m_framebuffers;
	
	void updateSwapchain();
	void updateFramebuffers(vk::RenderPass& renderPass, vk::ImageView& depthBufferView);
	vk::Image& getImage(uint32_t index);

	Swapchain(vk::PhysicalDevice& physicalDevice,
		vk::SurfaceKHR& surface,
		vk::Device& logicalDevice,
		vk::Queue& presentQueue);

	vk::Extent2D m_extent;
	vk::Format m_colorFormat{ vk::Format::eUndefined };

	uint32_t m_imageCount;

private:
	vk::PhysicalDevice& m_physicalDevice;
	vk::SurfaceKHR& m_surface;
	vk::Device& m_logicalDevice;
	vk::Queue& m_presentQueue;
	uint32_t m_presentQueueIndex;

	std::vector<vk::Image> m_images;
	std::vector<vk::UniqueImageView> m_imageViews;

	vk::ColorSpaceKHR m_colorSpace{ vk::ColorSpaceKHR::eSrgbNonlinear };

	vk::SwapchainCreateInfoKHR m_createInfo;
	vk::ImageViewCreateInfo m_imageViewCreateInfo;
};

