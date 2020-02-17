#pragma once

#include <vulkan/vulkan.hpp>

#include <string>

class LogicalDevice;
class Swapchain;
class RenderPass;

class PipelineLayout {
public:
    vk::PipelineLayout& get();

    PipelineLayout(LogicalDevice& logicalDevice);
    ~PipelineLayout();

private:
    vk::PipelineLayout m_pipelineLayout;

    LogicalDevice& m_logicalDevice;
};
