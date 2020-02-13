#pragma once

#include <vulkan/vulkan.hpp>

#include <string>

class VulkanShaderModule {
public:
    VulkanShaderModule(vk::Device device, const std::string& shaderPath);
    ~VulkanShaderModule();

private:
    vk::ShaderModule module;

    vk::Device& device;
};
