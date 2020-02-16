#pragma once

#include <vulkan/vulkan.hpp>

class LogicalDevice;

class CommandPool {
public:
    vk::CommandPool& get();

    CommandPool(LogicalDevice& device, uint32_t queueFamilyIndex);
    ~CommandPool();

private:
    vk::CommandPool m_commandPool;

    LogicalDevice& m_logicalDevice;
};