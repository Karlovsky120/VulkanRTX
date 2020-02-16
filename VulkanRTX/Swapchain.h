#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>

class LogicalDevice;
class PhysicalDevice;
class Surface;

class Swapchain {
public:
    vk::SwapchainKHR& get();

    vk::Extent2D& getExtent();
    vk::SurfaceFormatKHR getFormat();
    std::vector<vk::ImageView> getImageViews();

    Swapchain(LogicalDevice& logicalDevice, PhysicalDevice& physicalDevice, Surface& surface);
    ~Swapchain();

private:
    vk::SwapchainKHR m_swapchain;

    std::vector<vk::Image> m_images;
    std::vector<vk::ImageView> m_imageViews;

    vk::Extent2D m_extent;
    vk::SurfaceFormatKHR m_format;
    vk::PresentModeKHR m_presentMode;

    vk::SurfaceFormatKHR chooseFormat(std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR choosePresentMode(std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseExtent(vk::SurfaceCapabilitiesKHR& surfaceCapabilites, uint32_t width, uint32_t height);

    void createImageViews();

    LogicalDevice& m_logicalDevice;
};
