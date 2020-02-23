#pragma once

#include <vulkan/vulkan.hpp>

class Buffer;
class CommandBuffer;
class CommandPool;
class DeviceMemory;
class Framebuffers;
class PhysicalDevice;
class Pipeline;
class PipelineLayout;
class RenderPass;
class ShaderModule;
class Surface;
class Swapchain;

class LogicalDevice {
public:
    vk::Device& get();

    std::unique_ptr<Swapchain> createSwapchain(PhysicalDevice& physicalDevice, Surface& surface);
    std::unique_ptr<RenderPass> createRenderPass(vk::SurfaceFormatKHR& surfaceFormat);
    std::unique_ptr<ShaderModule> createShaderModule(const std::string shaderPath);
    std::unique_ptr<Pipeline> createPipeline(PipelineLayout& pipelineLayout, RenderPass& renderPass, Swapchain& swapchain);
    std::unique_ptr<PipelineLayout> createPipelineLayout();
    std::unique_ptr<Framebuffers> createFramebuffers(RenderPass& renderPass, Swapchain& swapchain);
    std::unique_ptr<CommandPool> createCommandPool(uint32_t queueFamilyIndex);
    std::unique_ptr<CommandBuffer> createCommandBuffer(CommandPool& commandPool);
    std::unique_ptr<Buffer> createBuffer(uint32_t size, vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryFlags);
    std::unique_ptr<DeviceMemory> allocateDeviceMemory(const uint32_t memoryType);

    vk::Queue& getGraphicsQueue();
    vk::Queue& getPresentQueue();
    vk::Queue& getTransferQueue();

    const uint32_t getGraphicsQueueIndex() const;
    const uint32_t getTransferQueueIndex() const;

    LogicalDevice(PhysicalDevice& physicalDevice, vk::SurfaceKHR& surface);

private:
    vk::UniqueDevice m_logicalDevice;

    uint32_t m_graphicsQueueIndex;
    uint32_t m_transferQueueIndex;
    uint32_t m_presentQueueIndex;

    vk::Queue m_graphicsQueue;
    vk::Queue m_transferQueue;
    vk::Queue m_presentQueue;

    const vk::PhysicalDevice& m_physicalDevice;
    vk::SurfaceKHR& m_surface;
};
