#pragma once

#include <vulkan/vulkan.hpp>

#include <string>

class ShaderModule {
public:
    vk::ShaderModule& get();

    ShaderModule(vk::Device& logicalDevice, const std::string shaderPath);

private:
    vk::UniqueShaderModule m_shaderModule;

    vk::Device& m_logicalDevice;
};
