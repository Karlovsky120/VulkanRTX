#pragma once

#include <vulkan/vulkan.hpp>

class CommandBuffer;
class CommandPool;
class Framebuffers;
class PhysicalDevice;
class Pipeline;
class RenderPass;
class ShaderModule;
class Surface;
class Swapchain;

class LogicalDevice {
public:
    vk::Device& get();
    std::unique_ptr<Swapchain> createSwapchain(PhysicalDevice& physicalDevice, Surface& surface);
    std::unique_ptr<RenderPass> createRenderPass(Swapchain& swapchain);
    std::unique_ptr<ShaderModule> createShaderModule(const std::string shaderPath);
    std::unique_ptr<Pipeline> createPipeline(Swapchain& swapchain);
    std::unique_ptr<Framebuffers> createFramebuffers(Swapchain& swapchain, RenderPass& renderPass);
    std::unique_ptr<CommandPool> createCommandPool(uint32_t queueFamilyIndex);
    std::unique_ptr<CommandBuffer> createCommandBuffer(CommandPool& commandPool);

    vk::Queue& getGraphicsQueue();
    vk::Queue& getPresentQueue();

    uint32_t getGraphicsQueueIndex();

    LogicalDevice(PhysicalDevice& physicalDevice, Surface& surface);
    ~LogicalDevice();

    LogicalDevice(const LogicalDevice&) = delete;
    LogicalDevice& operator=(const LogicalDevice&) = delete;

private:
    vk::Device m_logicalDevice;

    uint32_t m_graphicsQueueIndex;
    uint32_t m_transferQueueIndex;
    uint32_t m_presentQueueIndex;

    vk::Queue m_graphicsQueue;
    vk::Queue m_transferQueue;
    vk::Queue m_presentQueue;

    PhysicalDevice& m_physicalDevice;
    Surface& m_surface;
};
