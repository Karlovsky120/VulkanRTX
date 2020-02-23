#pragma once

#include <vulkan/vulkan.hpp>

#include <string>

class PipelineLayout {
public:
    vk::PipelineLayout& get();

    PipelineLayout(vk::Device& logicalDevice);

private:
    vk::UniquePipelineLayout m_pipelineLayout;

    vk::Device& m_logicalDevice;
};
