#pragma once

#include <vulkan/vulkan.hpp>

#include "VulkanRenderPass.h"

#include <string>

class VulkanSwapchain;

class VulkanPipeline {
public:
    vk::ShaderModule createShaderModule(const std::string& shaderPath);

    VulkanPipeline(vk::Device& device, VulkanSwapchain& swapchain);
    ~VulkanPipeline();

private:
    vk::PipelineLayout pipelineLayout;
    vk::Pipeline pipeline;

    std::vector<vk::ShaderModule> shaderModules;

    VulkanRenderPass renderPass;

    vk::Device& device;
};
