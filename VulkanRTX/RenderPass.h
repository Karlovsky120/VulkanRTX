#pragma once

#include <vulkan/vulkan.hpp>

class Swapchain;

class RenderPass {
public:
    vk::RenderPass& getRenderPass();

    RenderPass(vk::Device& device, Swapchain& swapchain);
    ~RenderPass();

private:
    vk::RenderPass renderPass;

    vk::Device& device;
};
