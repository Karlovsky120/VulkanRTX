#pragma once

#include <vulkan/vulkan.hpp>

class VulkanDevice {
public:
    vk::PhysicalDevice& getPhysicalDevice();
    vk::Device& getDevice();

    VulkanDevice(vk::Instance& instance, vk::SurfaceKHR& surface);
    ~VulkanDevice();

private:
    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    vk::Instance& instance;
    vk::SurfaceKHR& surface;

    vk::PhysicalDevice physicalDevice;
    vk::PhysicalDeviceProperties deviceProperties;
    vk::PhysicalDeviceFeatures deviceFeatures;
    vk::Device device;

    vk::Queue graphicsQueue;
    vk::Queue transferQueue;
    vk::Queue presentQueue;
};
