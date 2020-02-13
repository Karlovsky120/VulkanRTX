#pragma once

#include <vulkan/vulkan.hpp>

class VulkanDevice {
public:
    vk::Device& getDevice();

    VulkanDevice(vk::Instance& instance);
    ~VulkanDevice();

private:
    vk::Instance& instance;

    vk::PhysicalDevice physicalDevice;
    vk::PhysicalDeviceProperties deviceProperties;
    vk::PhysicalDeviceFeatures deviceFeatures;
    vk::Device device;

    uint32_t graphicsQueueIndex;
    uint32_t transferQueueIndex;
};
