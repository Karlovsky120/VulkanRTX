#include "Device.h"

vk::Device& Device::getDevice() {
    return device;
}

vk::PhysicalDevice& Device::getPhysicalDevice() {
    return physicalDevice;
}

vk::Queue& Device::getGraphicsQueue() {
    return graphicsQueue;
}

uint32_t Device::getGraphicsQueueIndex() {
    return graphicsQueueIndex;
}

Device::Device(vk::Instance& instance, vk::SurfaceKHR& surface) :
    instance(instance), surface(surface) {

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

    uint32_t queuesFound = 0;

    uint32_t i = 0;
    for (auto queueFamilyProperty : queueFamilyProperties) {
        if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eGraphics) {
            graphicsQueueIndex = i;
            ++queuesFound;
            break;
        }
    }

    i = 0;
    for (auto queueFamilyProperty : queueFamilyProperties) {
        if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eTransfer) {
            transferQueueIndex = i;
            ++queuesFound;
            break;
        }
    }

    i = 0;
    for (auto queueFamilyProperty : queueFamilyProperties) {
        if (physicalDevice.getSurfaceSupportKHR(i, surface)) {
            presentQueueIndex = i;
            ++queuesFound;
            break;
        }
    }

    if (queuesFound < 3) {
        throw std::runtime_error("Couldn't find suitable device queues!");
    }

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

    vk::DeviceQueueCreateInfo graphicsInfo;
    graphicsInfo.queueFamilyIndex = graphicsQueueIndex;
    graphicsInfo.queueCount = 1;
    float graphicsQueuePriority = 1.0f;
    graphicsInfo.pQueuePriorities = &graphicsQueuePriority;
    queueCreateInfos.push_back(graphicsInfo);

    if (graphicsQueueIndex != transferQueueIndex) {
        vk::DeviceQueueCreateInfo transferInfo;
        transferInfo.queueFamilyIndex = transferQueueIndex;
        transferInfo.queueCount = 1;
        float transferQueuePriority = 0.5f;
        transferInfo.pQueuePriorities = &transferQueuePriority;
        queueCreateInfos.push_back(transferInfo);
    }

    if (presentQueueIndex != graphicsQueueIndex && presentQueueIndex != transferQueueIndex) {
        vk::DeviceQueueCreateInfo presentInfo;
        presentInfo.queueFamilyIndex = presentQueueIndex;
        presentInfo.queueCount = 1;
        float presentQueuePriority = 0.75f;
        presentInfo.pQueuePriorities = &presentQueuePriority;
        queueCreateInfos.push_back(presentInfo);
    }

    vk::PhysicalDeviceFeatures deviceFeatures;

    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    device = physicalDevice.createDevice(deviceCreateInfo);;

    graphicsQueue = device.getQueue(graphicsQueueIndex, 0);
    transferQueue = device.getQueue(transferQueueIndex, 0);
    presentQueue = device.getQueue(presentQueueIndex, 0);
}

Device::~Device() {
    device.destroy();
}