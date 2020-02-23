#pragma once

#include <vulkan/vulkan.hpp>

class CommandPool;
class LogicalDevice;
class Swapchain;

class CommandBuffer {
public:
    vk::CommandBuffer& get();

    CommandBuffer(vk::Device& logicalDevice, vk::CommandPool& commandPool);

    void begin();
    void beginRenderPass(vk::RenderPass& renderPass, vk::Framebuffer& framebuffer, Swapchain& swapchain);

    void submit(vk::Queue& submitQueue,
                vk::Fence& fence,
                std::vector<vk::Semaphore> waitSemaphores,
                std::vector<vk::Semaphore> signalSemaphores);

private:
    vk::UniqueCommandBuffer m_commandBuffer;

    vk::CommandPool& m_commandPool;
    vk::Device& m_logicalDevice;
};
