#pragma once

#include "Device.h"

#include <vulkan/vulkan.hpp>

#include <vector>

class Swapchain {
public:
    vk::Extent2D& getExtent();
    vk::SurfaceFormatKHR getFormat();

    Swapchain(Device& vulkanDevice, vk::SurfaceKHR& surface, uint32_t width, uint32_t height);
    ~Swapchain();

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
