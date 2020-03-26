#include "CmdBufferAllocator.h"

void CmdBufferAllocator::init(std::shared_ptr<CmdBufferAllocator>& storage) {
    storage = std::make_shared<CmdBufferAllocator>();
    instance = storage;
}

std::shared_ptr<CmdBufferAllocator> CmdBufferAllocator::get() {
    return instance.lock();
}

std::vector<vk::CommandBuffer> CmdBufferAllocator::createBuffers(
    const vk::CommandPool& commandPool,
    const uint32_t count) {

    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.commandPool = commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = count;

    return m_context.lock()->m_logicalDevice->allocateCommandBuffers(allocInfo);
}

vk::CommandBuffer CmdBufferAllocator::createBuffer(
    const vk::CommandPool& commandPool) {

    return createBuffers(commandPool, 1)[0];
}

std::vector<vk::UniqueCommandBuffer> CmdBufferAllocator::createBuffersUnique(
    const vk::CommandPool& commandPool,
    const uint32_t count) {

    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.commandPool = commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = count;

    return m_context.lock()->m_logicalDevice->allocateCommandBuffersUnique(allocInfo);
}

vk::UniqueCommandBuffer CmdBufferAllocator::createBufferUnique(
    const vk::CommandPool& commandPool) {

    return std::move(createBuffersUnique(commandPool, 1)[0]);
}

void CmdBufferAllocator::submitBuffer(vk::CommandBuffer& cmdBuffer, vk::Queue& queue, bool wait) {
    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    vk::FenceCreateInfo info;
    vk::UniqueFence fence = m_context.lock()->m_logicalDevice->createFenceUnique(info);

    queue.submit(submitInfo, *fence);

    if (wait) {
        m_context.lock()->m_logicalDevice->waitForFences(*fence, VK_TRUE, UINT64_MAX);
    }
}

vk::UniqueCommandPool CmdBufferAllocator::createCommandPool(const uint32_t queueFamilyIndex) const {
    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.queueFamilyIndex = queueFamilyIndex;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    return m_context.lock()->m_logicalDevice->createCommandPoolUnique(poolInfo);
}

std::weak_ptr<CmdBufferAllocator> CmdBufferAllocator::instance;

CmdBufferAllocator::CmdBufferAllocator() :
    m_context(VulkanContext::get()) {

    m_graphicsCmdPool = createCommandPool(m_context.lock()->m_graphicsQueueIndex);
    m_computeCmdPool = createCommandPool(m_context.lock()->m_computeQueueIndex);
    m_transferCmdPool = createCommandPool(m_context.lock()->m_transferQueueIndex);
}