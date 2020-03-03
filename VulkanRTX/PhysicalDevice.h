#pragma once

#include <vulkan/vulkan.hpp>

class LogicalDevice;
class Surface;

class PhysicalDevice {
public:
    vk::PhysicalDevice& get();

    std::unique_ptr<LogicalDevice> createLogicalDevice(Surface& surface);

    const vk::PhysicalDeviceMemoryProperties& getMemoryProperties() const;

    PhysicalDevice(vk::Instance& instance);

private:
    vk::PhysicalDevice m_physicalDevice;

    vk::PhysicalDeviceProperties m_deviceProperties;
    vk::PhysicalDeviceFeatures m_deviceFeatures;
    vk::PhysicalDeviceMemoryProperties m_deviceMemoryProperties;
    vk::PhysicalDeviceRayTracingPropertiesNV m_rayTracingProperties;

    vk::Instance& m_instance;
};
