#pragma once

#include <vulkan/vulkan.hpp>

#include <string>

class LogicalDevice;

class Pipeline {
public:
    vk::Pipeline& get();

    Pipeline(LogicalDevice& logicalDevice,
             vk::PipelineLayout& pipelineLayout,
             vk::RenderPass& renderPass,
             vk::Extent2D& extent);

private:
    vk::UniquePipeline m_pipeline;

    vk::RenderPass& m_renderPass;
    vk::Device& m_logicalDevice;
};
