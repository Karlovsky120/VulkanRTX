#pragma once

#include <vulkan/vulkan.hpp>

class Device;

class CommandPool {
public:
    CommandPool(Device& device, uint32_t queueFamilyIndex);
    ~CommandPool();

private:
    vk::CommandPool commandPool;

    Device& device;
};