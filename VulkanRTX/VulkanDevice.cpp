#include "VulkanDevice.h"

vk::Device& VulkanDevice::getDevice() {
    return device;
}

VulkanDevice::VulkanDevice(vk::Instance& instance) :
    instance(instance) {

    std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();

    if (physicalDevices.size() == 0) {
        throw std::runtime_error("Failed to find GPU with Vulkan support!");
    }

    physicalDevice = physicalDevices[0];

    for (vk::PhysicalDevice currentPhysicalDevice : physicalDevices) {
        deviceProperties = physicalDevice.getProperties();
        deviceFeatures = physicalDevice.getFeatures();

        if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            physicalDevice = currentPhysicalDevice;
            break;
        }
    }

    auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    uint32_t i = 0;
    uint32_t queuesFound = 0;

    uint32_t graphicsQueueIndex;
    uint32_t transferQueueIndex;

    for (auto queueFamilyProperty : queueFamilyProperties) {
        if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eGraphics) {
            graphicsQueueIndex = i;
            ++queuesFound;
            break;
        }
    }

    for (auto queueFamilyProperty : queueFamilyProperties) {
        if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eTransfer) {
            transferQueueIndex = i;
            ++queuesFound;
            break;
        }
    }

    if (queuesFound < 2) {
        throw std::runtime_error("Couldn't find suitable device queues!");
    }

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

    uint32_t queueFamilyCount = 1;

    vk::DeviceQueueCreateInfo graphicsInfo;
    graphicsInfo.queueFamilyIndex = graphicsQueueIndex;
    graphicsInfo.queueCount = 1;
    float graphicsQueuePriority = 1.0f;
    graphicsInfo.pQueuePriorities = &graphicsQueuePriority;
    queueCreateInfos.push_back(graphicsInfo);

    if (graphicsQueueIndex != transferQueueIndex) {
        queueFamilyCount = 2;

        vk::DeviceQueueCreateInfo transferInfo;
        transferInfo.queueFamilyIndex = transferQueueIndex;
        transferInfo.queueCount = 1;
        float transferQueuePriority = 0.5f;
        transferInfo.pQueuePriorities = &transferQueuePriority;
        queueCreateInfos.push_back(transferInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures;

    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = queueFamilyCount;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    device = physicalDevice.createDevice(deviceCreateInfo);;

    graphicsQueue = device.getQueue(graphicsQueueIndex, 0);
    transferQueue = device.getQueue(transferQueueIndex, 0);
}

VulkanDevice::~VulkanDevice() {
    device.destroy();
}