#include "CommandBuffer.h"

vk::CommandBuffer& CommandBuffer::get() {
    return *m_commandBuffer;
}

CommandBuffer::CommandBuffer(vk::Device& logicalDevice, vk::CommandPool& commandPool) :
    m_logicalDevice(logicalDevice),
    m_commandPool(commandPool) {

    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.commandPool = commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    m_commandBuffer = std::move(logicalDevice.allocateCommandBuffersUnique(allocInfo)[0]);
}