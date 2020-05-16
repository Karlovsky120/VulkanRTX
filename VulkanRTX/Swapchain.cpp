#include "Swapchain.h"

void Swapchain::updateSwapchain() {
	const vk::SwapchainKHR oldSwapchain = *m_swapchain;

	vk::SurfaceCapabilitiesKHR surfaceCapabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(m_surface);
	if (surfaceCapabilities.currentExtent.width != -1) {
		m_extent = surfaceCapabilities.currentExtent;
	}

	uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount) {
		imageCount = surfaceCapabilities.maxImageCount;
	}

	if (imageCount != m_imageCount) {
		throw std::runtime_error("Dynamic swapchain image number is not supported");
	}

	std::vector<vk::SurfaceFormatKHR> surfaceFormats = m_physicalDevice.getSurfaceFormatsKHR(m_surface);

	m_colorFormat = surfaceFormats[0].format;
	m_colorSpace = surfaceFormats[0].colorSpace;
	for (const auto& availableFormat : surfaceFormats) {
		if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			m_colorFormat = vk::Format::eB8G8R8A8Srgb;
			m_colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
			break;
		}
	}

	std::vector<vk::PresentModeKHR> presentModes = m_physicalDevice.getSurfacePresentModesKHR(m_surface);
	vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;
	for (const auto& presentMode : presentModes) {
		if (presentMode == vk::PresentModeKHR::eMailbox) {
			swapchainPresentMode = vk::PresentModeKHR::eMailbox;
			break;
		}
	}

	m_createInfo.minImageCount = imageCount;
	m_createInfo.imageFormat = m_colorFormat;
	m_createInfo.imageColorSpace = m_colorSpace;
	m_createInfo.imageExtent = vk::Extent2D(m_extent.width, m_extent.height);
	m_createInfo.preTransform = surfaceCapabilities.currentTransform;
	m_createInfo.oldSwapchain = oldSwapchain;

	m_swapchain = m_logicalDevice.createSwapchainKHRUnique(m_createInfo);

	if (oldSwapchain) {
		m_imageViews.clear();
	}

	m_imageViewCreateInfo.format = m_colorFormat;

	m_images = m_logicalDevice.getSwapchainImagesKHR(*m_swapchain);
	for (vk::Image& image : m_images) {
		m_imageViewCreateInfo.image = image;
		m_imageViews.push_back(m_logicalDevice.createImageViewUnique(m_imageViewCreateInfo));
	}
}

void Swapchain::updateFramebuffers(vk::RenderPass& renderPass, vk::ImageView& depthBufferView) {
	m_framebuffers.clear();

	vk::FramebufferCreateInfo createInfo;
	createInfo.renderPass = renderPass;
	createInfo.attachmentCount = 2;
	createInfo.width = m_extent.width;
	createInfo.height = m_extent.height;
	createInfo.layers = 1;

	std::vector<vk::ImageView> imageViews = std::vector<vk::ImageView>(2);
	imageViews[1] = depthBufferView;

	for (vk::UniqueImageView& imageView : m_imageViews) {
		imageViews[0] = *imageView,
		createInfo.pAttachments = imageViews.data();
		m_framebuffers.push_back(m_logicalDevice.createFramebufferUnique(createInfo));
	}
}

vk::Image& Swapchain::getImage(uint32_t index) {
	return m_images[index];
}

Swapchain::Swapchain(vk::PhysicalDevice& physicalDevice,
	vk::SurfaceKHR& surface,
	vk::Device& logicalDevice,
	vk::Queue& presentQueue) :
	m_physicalDevice(physicalDevice),
	m_surface(surface),
	m_logicalDevice(logicalDevice),
	m_presentQueue(presentQueue) {

	m_createInfo.surface = m_surface;
	m_createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
	m_createInfo.imageArrayLayers = 1;
	m_createInfo.imageSharingMode = vk::SharingMode::eExclusive;
	m_createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	m_createInfo.clipped = VK_TRUE;

	m_imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
	m_imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	m_imageViewCreateInfo.subresourceRange.levelCount = 1;
	m_imageViewCreateInfo.subresourceRange.layerCount = 1;

	vk::SurfaceCapabilitiesKHR surfaceCapabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(m_surface);

	m_imageCount = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0 && m_imageCount > surfaceCapabilities.maxImageCount) {
		m_imageCount = surfaceCapabilities.maxImageCount;
	}

	updateSwapchain();
}