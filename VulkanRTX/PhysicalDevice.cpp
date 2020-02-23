#include "PhysicalDevice.h"

#include "LogicalDevice.h"
#include "Surface.h"

vk::PhysicalDevice& PhysicalDevice::get() {
    return m_physicalDevice;
}

std::unique_ptr<LogicalDevice> PhysicalDevice::createLogicalDevice(Surface& surface) {
    return std::make_unique<LogicalDevice>(*this, surface.get());
}

const vk::PhysicalDeviceMemoryProperties& PhysicalDevice::getMemoryProperties() const {
    return m_deviceMemoryProperties;
}

PhysicalDevice::PhysicalDevice(vk::Instance& instance) :
    m_instance(instance) {

    std::vector<vk::PhysicalDevice> physicalDevices = m_instance.enumeratePhysicalDevices();

    if (physicalDevices.size() == 0) {
        throw std::runtime_error("Failed to find GPU with Vulkan support!");
    }

    m_physicalDevice = physicalDevices[0];

    for (vk::PhysicalDevice currentPhysicalDevice : physicalDevices) {
        m_deviceProperties = m_physicalDevice.getProperties();
        m_deviceFeatures = m_physicalDevice.getFeatures();
        m_deviceMemoryProperties = m_physicalDevice.getMemoryProperties();

        if (m_deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            m_physicalDevice = currentPhysicalDevice;
            break;
        }
    }
}