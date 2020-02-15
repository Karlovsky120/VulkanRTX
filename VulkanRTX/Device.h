#pragma once

#include <vulkan/vulkan.hpp>

class Device {
public:
    vk::PhysicalDevice& getPhysicalDevice();
    vk::Device& getDevice();

    vk::Queue& getGraphicsQueue();
    uint32_t getGraphicsQueueIndex();

    Device(vk::Instance& instance, vk::SurfaceKHR& surface);
    ~Device();

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

    uint32_t graphicsQueueIndex;
    uint32_t transferQueueIndex;
    uint32_t presentQueueIndex;

    vk::Queue graphicsQueue;
    vk::Queue transferQueue;
    vk::Queue presentQueue;
};
