#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>

class CommandPool;
class LogicalDevice;
class Swapchain;

class CommandBuffer {
public:
    vk::CommandBuffer& get();

    CommandBuffer(LogicalDevice& device, CommandPool& commandPool);
    ~CommandBuffer();

    void begin();
    void beginRenderPass(vk::RenderPass& renderPass, vk::Framebuffer& framebuffer, Swapchain& swapchain);

private:
    vk::CommandBuffer m_commandBuffer;

    CommandPool& m_commandPool;
    LogicalDevice& m_logicalDevice;
};
