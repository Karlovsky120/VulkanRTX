#pragma once

#include <vulkan/vulkan.hpp>

class PipelineLayout {
public:
    vk::PipelineLayout& get();

    PipelineLayout(vk::Device& logicalDevice, vk::DescriptorSetLayout setLayout);

private:
    vk::UniquePipelineLayout m_pipelineLayout;

    vk::Device& m_logicalDevice;
};
