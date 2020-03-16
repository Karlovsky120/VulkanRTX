#include "CmdBufferAllocator.h"

void CmdBufferAllocator::init(VulkanContext* vkCtx) {
    instance = new CmdBufferAllocator(vkCtx);
}

CmdBufferAllocator* CmdBufferAllocator::get() {
    return instance;
}

vk::UniqueCommandBuffer CmdBufferAllocator::getTransferBuffer() {
    return createBuffer(*m_transferCmdPool);
}

std::vector<vk::UniqueCommandBuffer> CmdBufferAllocator::getTransferBuffers(const uint32_t count) {
    return createBuffers(*m_transferCmdPool, count);
}

vk::UniqueCommandBuffer CmdBufferAllocator::getGraphicsBuffer() {
    return createBuffer(*m_graphicsCmdPool);
}

std::vector<vk::UniqueCommandBuffer> CmdBufferAllocator::getGraphicsBuffers(const uint32_t count) {
    return createBuffers(*m_graphicsCmdPool, count);
}

void CmdBufferAllocator::submitBuffer(vk::CommandBuffer& cmdBuffer, bool wait) {
    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    bufferToQueue.find(cmdBuffer)->second->submit(submitInfo, nullptr);

    if (wait) {
        m_context->m_logicalDevice->waitIdle();
    }
}

CmdBufferAllocator* CmdBufferAllocator::instance = nullptr;

CmdBufferAllocator::CmdBufferAllocator(VulkanContext* vkCtx) :
    m_context(vkCtx) {
    
    m_transferCmdPool = createCommandPool(m_context->m_transferQueueIndex);
    poolToQueue.insert(std::pair<const vk::CommandPool*, const vk::Queue*>(&*m_transferCmdPool, &m_context->m_transferQueue));

    m_graphicsCmdPool = createCommandPool(m_context->m_graphicsQueueIndex);
    poolToQueue.insert(std::pair<const vk::CommandPool*, const vk::Queue*>(&*m_graphicsCmdPool, &m_context->m_graphicsQueue));
}

vk::UniqueCommandPool CmdBufferAllocator::createCommandPool(const uint32_t queueFamilyIndex) const {
    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.queueFamilyIndex = queueFamilyIndex;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    return m_context->m_logicalDevice->createCommandPoolUnique(poolInfo);
}

std::vector<vk::UniqueCommandBuffer> CmdBufferAllocator::createBuffers(
    const vk::CommandPool& commandPool,
    const uint32_t count) {

    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.commandPool = commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = count;

    std::vector<vk::UniqueCommandBuffer> buffers = m_context->m_logicalDevice->allocateCommandBuffersUnique(allocInfo);

    for (const vk::UniqueCommandBuffer& buffer : buffers) {
        bufferToQueue.insert(
            std::pair<const vk::CommandBuffer, const vk::Queue*>(
                *buffer,
                poolToQueue.find(&commandPool)->second));
    }

    return std::move(buffers);
}

vk::UniqueCommandBuffer CmdBufferAllocator::createBuffer(
    const vk::CommandPool& commandPool) {

    return std::move(createBuffers(commandPool, 1)[0]);
}