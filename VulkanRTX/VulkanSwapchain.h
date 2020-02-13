#pragma once

#include "VulkanDevice.h"

#include <vulkan/vulkan.hpp>

#include <vector>

class VulkanSwapchain {
public:
    VulkanSwapchain(VulkanDevice& vulkanDevice, vk::SurfaceKHR& surface, uint32_t width, uint32_t height);
    ~VulkanSwapchain();

private:
    vk::SwapchainKHR swapchain;
    std::vector<vk::Image> swapchainImages;
    std::vector<vk::ImageView> swapchainImageViews;

    vk::SurfaceFormatKHR chooseFormat(std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR choosePresentMode(std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseExtent(vk::SurfaceCapabilitiesKHR& surfaceCapabilites, uint32_t width, uint32_t height);

    void createImageViews();

    vk::SurfaceFormatKHR format;
    vk::PresentModeKHR presentMode;

    vk::Extent2D extent;

    vk::Device& device;
};
