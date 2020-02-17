#pragma once

#include <vulkan/vulkan.hpp>

#include <string>

class LogicalDevice;
class PipelineLayout;
class RenderPass;
class Swapchain;

class Pipeline {
public:
    vk::Pipeline& get();

    Pipeline(LogicalDevice& logicalDevice, PipelineLayout& pipelineLayout, RenderPass& renderPass, Swapchain& swapchain);
    ~Pipeline();

private:
    vk::Pipeline m_pipeline;

    RenderPass& m_renderPass;
    LogicalDevice& m_logicalDevice;
};
