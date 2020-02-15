#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>

class Device;
class Swapchain;

class CommandBuffer {
public:
    CommandBuffer(vk::Device& device, vk::CommandPool& commandPool);

    void begin();
    void beginRenderPass(vk::RenderPass& renderPass, vk::Framebuffer& framebuffer, Swapchain& swapchain);

private:
    vk::CommandBuffer commandBuffer;

    vk::Device& device;
};
