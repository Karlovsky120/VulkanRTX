#pragma once

#include <vulkan/vulkan.hpp>

#include <string>

class ShaderModule {
public:
    ShaderModule(vk::Device device, const std::string& shaderPath);
    ~ShaderModule();

private:
    vk::ShaderModule module;

    vk::Device& device;
};
