#pragma once

#include <vulkan/vulkan.hpp>

#include "RenderPass.h"

#include <string>

class Swapchain;

class Pipeline {
public:
    vk::ShaderModule createShaderModule(const std::string& shaderPath);

    Pipeline(vk::Device& device, Swapchain& swapchain);
    ~Pipeline();

private:
    vk::PipelineLayout pipelineLayout;
    vk::Pipeline pipeline;

    std::vector<vk::ShaderModule> shaderModules;

    RenderPass renderPass;

    vk::Device& device;
};
