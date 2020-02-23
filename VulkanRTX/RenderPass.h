#pragma once

#include <vulkan/vulkan.hpp>

class LogicalDevice;
class Swapchain;

class RenderPass {
public:
    vk::RenderPass& get();

    RenderPass(vk::Device& logicalDevice, vk::SurfaceFormatKHR& surfaceFormat);

private:
    vk::UniqueRenderPass m_renderPass;

    vk::Device& m_logicalDevice;
};
