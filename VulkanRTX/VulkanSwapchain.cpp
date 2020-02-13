#include "VulkanSwapchain.h"

vk::SurfaceFormatKHR VulkanSwapchain::chooseFormat(std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR VulkanSwapchain::choosePresentMode(std::vector<vk::PresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            presentMode = availablePresentMode;
        }
    }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D VulkanSwapchain::chooseExtent(vk::SurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }

    vk::Extent2D actualExtent = {width, height};

    actualExtent.width = (std::max)(capabilities.minImageExtent.width, (std::min)(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = (std::max)(capabilities.minImageExtent.height, (std::min)(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
}

VulkanSwapchain::VulkanSwapchain(VulkanDevice& vulkanDevice, vk::SurfaceKHR& surface, uint32_t width, uint32_t height) :
    device(vulkanDevice.getDevice()) {

    vk::PhysicalDevice physicalDevice = vulkanDevice.getPhysicalDevice();

    vk::SurfaceCapabilitiesKHR capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);

    std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(surface);
    std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR(surface);

    format = chooseFormat(availableFormats);
    presentMode = choosePresentMode(availablePresentModes);
    extent = chooseExtent(capabilities, width, height);

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = format.format;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = nullptr;

    swapchain = device.createSwapchainKHR(createInfo);

    swapchainImages = device.getSwapchainImagesKHR(swapchain);
}

VulkanSwapchain::~VulkanSwapchain() {
    device.destroySwapchainKHR(swapchain);
}