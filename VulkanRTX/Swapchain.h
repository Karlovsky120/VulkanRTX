#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>

class Surface;

class Swapchain {
public:
    vk::SwapchainKHR& get();

    vk::Extent2D& getExtent();
    vk::SurfaceFormatKHR& getFormat();
    std::vector<vk::ImageView> getImageViews();

    Swapchain(vk::Device& logicalDevice, vk::PhysicalDevice& physicalDevice, Surface& surface);
    ~Swapchain();

private:
    vk::UniqueSwapchainKHR m_swapchain;

    std::vector<vk::Image> m_images;
    std::vector<vk::ImageView> m_imageViews;

    vk::Extent2D m_extent;
    vk::SurfaceFormatKHR m_format;
    vk::PresentModeKHR m_presentMode;

    vk::SurfaceFormatKHR chooseFormat(std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR choosePresentMode(std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseExtent(vk::SurfaceCapabilitiesKHR& surfaceCapabilites);

    void createImageViews();

    Surface& m_surface;
    vk::Device& m_logicalDevice;
};
