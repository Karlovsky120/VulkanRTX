#include "LogicalDevice.h"

#include "CommandBuffer.h"
#include "CommandPool.h"
#include "Framebuffers.h"
#include "PhysicalDevice.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "ShaderModule.h"
#include "Surface.h"
#include "Swapchain.h"

vk::Device& LogicalDevice::get() {
    return m_logicalDevice;
}

std::unique_ptr<Swapchain> LogicalDevice::createSwapchain(PhysicalDevice& physicalDevice, Surface& surface) {
    return std::make_unique<Swapchain>(*this, physicalDevice, surface);
}

std::unique_ptr<RenderPass> LogicalDevice::createRenderPass(Swapchain& swapchain) {
    return std::make_unique<RenderPass>(*this, swapchain);
}

std::unique_ptr<ShaderModule> LogicalDevice::createShaderModule(const std::string shaderPath) {
    return std::make_unique<ShaderModule>(*this, shaderPath);
}

std::unique_ptr<Pipeline> LogicalDevice::createPipeline(Swapchain& swapchain) {
    return std::make_unique<Pipeline>(*this, swapchain);
}

std::unique_ptr<Framebuffers> LogicalDevice::createFramebuffers(Swapchain& swapchain, RenderPass& renderPass) {
    return std::make_unique<Framebuffers>(*this, swapchain, renderPass);
}

std::unique_ptr<CommandPool> LogicalDevice::createCommandPool(uint32_t queueFamilyIndex) {
    return std::make_unique<CommandPool>(*this, queueFamilyIndex);
}

std::unique_ptr<CommandBuffer> LogicalDevice::createCommandBuffer(CommandPool& commandPool) {
    return std::make_unique<CommandBuffer>(*this, commandPool);
}

vk::Queue& LogicalDevice::getGraphicsQueue() {
    return m_graphicsQueue;
}

uint32_t LogicalDevice::getGraphicsQueueIndex() {
    return m_graphicsQueueIndex;
}

LogicalDevice::LogicalDevice(PhysicalDevice& physicalDevice, Surface& surface) :
    m_physicalDevice(physicalDevice),
    m_surface(surface) {

    auto queueFamilyProperties = physicalDevice.get().getQueueFamilyProperties();

    uint32_t queuesFound = 0;

    uint32_t i = 0;
    for (auto queueFamilyProperty : queueFamilyProperties) {
        if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eGraphics) {
            m_graphicsQueueIndex = i;
            ++queuesFound;
            break;
        }
    }

    i = 0;
    for (auto queueFamilyProperty : queueFamilyProperties) {
        if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eTransfer) {
            m_transferQueueIndex = i;
            ++queuesFound;
            break;
        }
    }

    i = 0;
    for (auto queueFamilyProperty : queueFamilyProperties) {
        if (physicalDevice.get().getSurfaceSupportKHR(i, surface.get())) {
            m_presentQueueIndex = i;
            ++queuesFound;
            break;
        }
    }

    if (queuesFound < 3) {
        throw std::runtime_error("Couldn't find suitable device queues!");
    }

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

    vk::DeviceQueueCreateInfo graphicsInfo;
    graphicsInfo.queueFamilyIndex = m_graphicsQueueIndex;
    graphicsInfo.queueCount = 1;
    float graphicsQueuePriority = 1.0f;
    graphicsInfo.pQueuePriorities = &graphicsQueuePriority;
    queueCreateInfos.push_back(graphicsInfo);

    if (m_graphicsQueueIndex != m_transferQueueIndex) {
        vk::DeviceQueueCreateInfo transferInfo;
        transferInfo.queueFamilyIndex = m_transferQueueIndex;
        transferInfo.queueCount = 1;
        float transferQueuePriority = 0.5f;
        transferInfo.pQueuePriorities = &transferQueuePriority;
        queueCreateInfos.push_back(transferInfo);
    }

    if (m_presentQueueIndex != m_graphicsQueueIndex && m_presentQueueIndex != m_transferQueueIndex) {
        vk::DeviceQueueCreateInfo presentInfo;
        presentInfo.queueFamilyIndex = m_presentQueueIndex;
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

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    m_logicalDevice = physicalDevice.get().createDevice(deviceCreateInfo);;

    m_graphicsQueue = m_logicalDevice.getQueue(m_graphicsQueueIndex, 0);
    m_transferQueue = m_logicalDevice.getQueue(m_transferQueueIndex, 0);
    m_presentQueue = m_logicalDevice.getQueue(m_presentQueueIndex, 0);
}

LogicalDevice::~LogicalDevice() {
    m_logicalDevice.destroy();
}