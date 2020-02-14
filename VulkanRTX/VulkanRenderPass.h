#pragma once

#include <vulkan/vulkan.hpp>

class VulkanSwapchain;

class VulkanRenderPass {
public:
    vk::RenderPass& getRenderPass();

    VulkanRenderPass(vk::Device& device, VulkanSwapchain& swapchain);
    ~VulkanRenderPass();

private:
    vk::RenderPass renderPass;

    vk::Device& device;
};
