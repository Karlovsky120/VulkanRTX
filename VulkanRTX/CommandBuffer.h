#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>

class CommandPool;
class LogicalDevice;
class Swapchain;

class CommandBuffer {
public:
    CommandBuffer(LogicalDevice& device, CommandPool& commandPool);

    void begin();
    void beginRenderPass(vk::RenderPass& renderPass, vk::Framebuffer& framebuffer, Swapchain& swapchain);

private:
    vk::CommandBuffer commandBuffer;

    LogicalDevice& m_logicalDevice;
};
