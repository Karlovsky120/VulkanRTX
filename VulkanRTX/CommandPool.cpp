#include "CommandPool.h"

#include "Device.h"

vk::CommandPool& CommandPool::getCommandPool() {
    return commandPool;
}

CommandPool::CommandPool(Device& device, uint32_t queueFamilyIndex) :
    device(device) {

    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    commandPool = device.getDevice().createCommandPool(poolInfo);
}

CommandPool::~CommandPool() {
    device.getDevice().destroyCommandPool(commandPool);
}