#pragma once

#include <vulkan/vulkan.hpp>

class LogicalDevice;
class Swapchain;

class RenderPass {
public:
    vk::RenderPass& get();

    RenderPass(LogicalDevice& logicalDevice, Swapchain& swapchain);
    ~RenderPass();

private:
    vk::RenderPass m_renderPass;

    LogicalDevice& m_logicalDevice;
};
