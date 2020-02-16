#pragma once

#include <vulkan/vulkan.hpp>

#include <string>

class LogicalDevice;

class ShaderModule {
public:
    vk::ShaderModule& get();

    ShaderModule(LogicalDevice& logicalDevice, const std::string shaderPath);
    ~ShaderModule();

private:
    vk::ShaderModule m_shaderModule;

    LogicalDevice& m_logicalDevice;
};
