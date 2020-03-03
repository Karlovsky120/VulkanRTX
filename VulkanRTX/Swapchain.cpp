#include "Swapchain.h"

#include "Surface.h"

#include <GLFW/glfw3.h>

vk::SwapchainKHR& Swapchain::get() {
    return *m_swapchain;
}

vk::Extent2D& Swapchain::getExtent() {
    return m_extent;
}

vk::SurfaceFormatKHR& Swapchain::getFormat() {
    return m_format;
}

std::vector<vk::ImageView> Swapchain::getImageViews() {
    return m_imageViews;
}

vk::SurfaceFormatKHR Swapchain::chooseFormat(std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR Swapchain::choosePresentMode(std::vector<vk::PresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            m_presentMode = availablePresentMode;
        }
    }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Swapchain::chooseExtent(vk::SurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }

    int width;
    int height;
    glfwGetFramebufferSize(m_surface.getWindow(), &width, &height);

    vk::Extent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    actualExtent.width = (std::max)(capabilities.minImageExtent.width, (std::min)(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = (std::max)(capabilities.minImageExtent.height, (std::min)(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
}

void Swapchain::createImageViews() {
    for (size_t i = 0; i < m_images.size(); ++i) {
        vk::ImageViewCreateInfo createInfo;
        createInfo.image = m_images[i];
        createInfo.viewType = vk::ImageViewType::e2D;
        createInfo.format = m_format.format;

        createInfo.components.r = vk::ComponentSwizzle::eIdentity;
        createInfo.components.g = vk::ComponentSwizzle::eIdentity;
        createInfo.components.b = vk::ComponentSwizzle::eIdentity;
        createInfo.components.a = vk::ComponentSwizzle::eIdentity;

        createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        m_imageViews.push_back(m_logicalDevice.createImageView(createInfo));
    }
}

Swapchain::Swapchain(vk::Device& logicalDevice, vk::PhysicalDevice& physicalDevice, Surface& surface) :
    m_logicalDevice(logicalDevice),
    m_surface(surface) {

    vk::SurfaceKHR vkSurface = surface.get();
    vk::SurfaceCapabilitiesKHR capabilities = physicalDevice.getSurfaceCapabilitiesKHR(vkSurface);

    std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(vkSurface);
    std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR(vkSurface);

    m_format = chooseFormat(availableFormats);
    m_presentMode = choosePresentMode(availablePresentModes);
    m_extent = chooseExtent(capabilities);

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.surface = vkSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = m_format.format;
    createInfo.imageColorSpace = m_format.colorSpace;
    createInfo.imageExtent = m_extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = m_presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = nullptr;

    m_swapchain = m_logicalDevice.createSwapchainKHRUnique(createInfo);
    m_images = m_logicalDevice.getSwapchainImagesKHR(*m_swapchain);

    createImageViews();
}

Swapchain::~Swapchain() {
    for (const auto& imageView : m_imageViews) {
        m_logicalDevice.destroyImageView(imageView);
    }
}