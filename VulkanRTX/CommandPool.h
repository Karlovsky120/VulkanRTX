#pragma once

#include <vulkan/vulkan.hpp>

class CommandPool {
public:
    vk::CommandPool& get();

    CommandPool(vk::Device& device, const uint32_t queueFamilyIndex);

private:
    vk::UniqueCommandPool m_commandPool;

    vk::Device& m_logicalDevice;
};