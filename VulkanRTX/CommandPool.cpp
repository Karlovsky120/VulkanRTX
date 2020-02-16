#include "CommandPool.h"

#include "LogicalDevice.h"

vk::CommandPool& CommandPool::get() {
    return m_commandPool;
}

CommandPool::CommandPool(LogicalDevice& logicalDevice, uint32_t queueFamilyIndex) :
    m_logicalDevice(logicalDevice) {

    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    m_commandPool = logicalDevice.get().createCommandPool(poolInfo);
}

CommandPool::~CommandPool() {
    m_logicalDevice.get().destroyCommandPool(m_commandPool);
}