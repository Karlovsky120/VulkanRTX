#pragma once

#include <vulkan/vulkan.hpp>

class CommandPool;

class CommandBuffer {
public:
    vk::CommandBuffer& get();

    CommandBuffer(vk::Device& logicalDevice, vk::CommandPool& commandPool);

private:
    vk::UniqueCommandBuffer m_commandBuffer;

    vk::CommandPool& m_commandPool;
    vk::Device& m_logicalDevice;
};
