#pragma once

#include <vulkan/vulkan.hpp>

class Instance;
class LogicalDevice;
class Surface;

class PhysicalDevice {
public:
    vk::PhysicalDevice& get();

    std::unique_ptr<LogicalDevice> createLogicalDevice(Surface& surface);

    PhysicalDevice(Instance& instance);

    PhysicalDevice(const PhysicalDevice&) = delete;
    PhysicalDevice& operator=(const PhysicalDevice&) = delete;

private:
    vk::PhysicalDevice m_physicalDevice;

    vk::PhysicalDeviceProperties m_deviceProperties;
    vk::PhysicalDeviceFeatures m_deviceFeatures;

    Instance& m_instance;
};
