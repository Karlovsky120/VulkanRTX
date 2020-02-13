#pragma once

#include <vulkan/vulkan.hpp>

class VulkanDevice {
public:
    vk::Device& getDevice();

    VulkanDevice(vk::Instance& instance, vk::SurfaceKHR& surface);
    ~VulkanDevice();

private:
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
