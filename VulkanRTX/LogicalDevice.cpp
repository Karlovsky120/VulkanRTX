#include "LogicalDevice.h"

#include "Buffer.h"
#include "CommandBuffer.h"
#include "CommandPool.h"
#include "DescriptorPool.h"
#include "DescriptorSets.h"
#include "DescriptorSetLayout.h"
#include "DeviceMemory.h"
#include "Framebuffers.h"
#include "PhysicalDevice.h"
#include "Pipeline.h"
#include "PipelineLayout.h"
#include "RenderPass.h"
#include "ShaderModule.h"
#include "Surface.h"
#include "Swapchain.h"

vk::Device& LogicalDevice::get() {
    return *m_logicalDevice;
}

std::unique_ptr<Swapchain> LogicalDevice::createSwapchain(PhysicalDevice& physicalDevice, Surface& surface) {
    return std::make_unique<Swapchain>(*m_logicalDevice, physicalDevice.get(), surface);
}

std::unique_ptr<RenderPass> LogicalDevice::createRenderPass(vk::SurfaceFormatKHR& surfaceFormat) {
    return std::make_unique<RenderPass>(*m_logicalDevice, surfaceFormat);
}

std::unique_ptr<ShaderModule> LogicalDevice::createShaderModule(const std::string shaderPath) {
    return std::make_unique<ShaderModule>(*m_logicalDevice, shaderPath);
}

std::unique_ptr<DescriptorSetLayout> LogicalDevice::createDescriptorSetLayout() {
    return std::make_unique<DescriptorSetLayout>(*m_logicalDevice);
}

std::unique_ptr<DescriptorPool> LogicalDevice::createDescriptorPool(uint32_t descriptorCount) {
    return std::make_unique<DescriptorPool>(*m_logicalDevice, descriptorCount);
}

std::unique_ptr<DescriptorSets> LogicalDevice::allocateDescriptorSets(vk::DescriptorPool& descriptorPool, std::vector<vk::DescriptorSetLayout> setLayouts) {
    return std::make_unique<DescriptorSets>(*m_logicalDevice, descriptorPool, setLayouts);
}

std::unique_ptr<PipelineLayout> LogicalDevice::createPipelineLayout(vk::DescriptorSetLayout setLayout) {
    return std::make_unique<PipelineLayout>(*m_logicalDevice, setLayout);
}

std::unique_ptr<Pipeline> LogicalDevice::createPipeline(PipelineLayout& pipelineLayout, RenderPass& renderPass, Swapchain& swapchain) {
    return std::make_unique<Pipeline>(*this, pipelineLayout.get(), renderPass.get(), swapchain.getExtent());
}

std::unique_ptr<Framebuffers> LogicalDevice::createFramebuffers(RenderPass& renderPass, Swapchain& swapchain) {
    return std::make_unique<Framebuffers>(*m_logicalDevice, renderPass.get(), swapchain);
}

std::unique_ptr<CommandPool> LogicalDevice::createCommandPool(uint32_t queueFamilyIndex) {
    return std::make_unique<CommandPool>(*m_logicalDevice, queueFamilyIndex);
}

std::unique_ptr<CommandBuffer> LogicalDevice::createCommandBuffer(CommandPool& commandPool) {
    return std::make_unique<CommandBuffer>(*m_logicalDevice, commandPool.get());
}

std::unique_ptr<Buffer> LogicalDevice::createBuffer(uint32_t size, vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryFlags) {
    return std::make_unique<Buffer>(*m_logicalDevice, size, usageFlags, memoryFlags);
}

std::unique_ptr<DeviceMemory> LogicalDevice::allocateDeviceMemory(uint32_t memoryType) {
    return std::make_unique<DeviceMemory>(*m_logicalDevice, memoryType);
}

vk::Queue& LogicalDevice::getGraphicsQueue() {
    return m_graphicsQueue;
}

vk::Queue& LogicalDevice::getPresentQueue() {
    return m_presentQueue;
}

vk::Queue& LogicalDevice::getTransferQueue() {
    return m_transferQueue;
}

const uint32_t LogicalDevice::getGraphicsQueueIndex() const {
    return m_graphicsQueueIndex;
}

const uint32_t LogicalDevice::getTransferQueueIndex() const {
    return m_transferQueueIndex;
}

LogicalDevice::LogicalDevice(PhysicalDevice& physicalDevice, vk::SurfaceKHR& surface) :
    m_physicalDevice(physicalDevice.get()),
    m_surface(surface) {

    auto queueFamilyProperties = m_physicalDevice.getQueueFamilyProperties();

    uint32_t queuesFound = 0;

    uint32_t i = 0;
    for (auto& queueFamilyProperty : queueFamilyProperties) {
        if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eGraphics) {
            m_graphicsQueueIndex = i;
            ++queuesFound;
            break;
        }
    }

    i = 0;
    for (auto& queueFamilyProperty : queueFamilyProperties) {
        if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eTransfer) {
            m_transferQueueIndex = i;
            ++queuesFound;
            break;
        }
    }

    i = 0;
    for (auto& queueFamilyProperty : queueFamilyProperties) {
        if (m_physicalDevice.getSurfaceSupportKHR(i, m_surface)) {
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

    m_logicalDevice = m_physicalDevice.createDeviceUnique(deviceCreateInfo);;

    m_graphicsQueue = m_logicalDevice->getQueue(m_graphicsQueueIndex, 0);
    m_transferQueue = m_logicalDevice->getQueue(m_transferQueueIndex, 0);
    m_presentQueue = m_logicalDevice->getQueue(m_presentQueueIndex, 0);
}