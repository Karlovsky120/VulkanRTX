#pragma once

#include <vulkan/vulkan.hpp>

#include <string>

class LogicalDevice;
class Swapchain;
class RenderPass;

class Pipeline {
public:
    vk::Pipeline& get();

    Pipeline(LogicalDevice& logicalDevice, Swapchain& swapchain);
    ~Pipeline();

private:
    vk::PipelineLayout m_pipelineLayout;
    vk::Pipeline m_pipeline;

    std::unique_ptr<RenderPass> m_renderPass;

    LogicalDevice& m_logicalDevice;
};
