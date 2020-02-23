#include "CommandPool.h"

#include "LogicalDevice.h"

vk::CommandPool& CommandPool::get() {
    return *m_commandPool;
}

CommandPool::CommandPool(vk::Device& logicalDevice, const uint32_t queueFamilyIndex) :
    m_logicalDevice(logicalDevice) {

    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    m_commandPool = logicalDevice.createCommandPoolUnique(poolInfo);
}