#include "PipelineLayout.h"

vk::PipelineLayout& PipelineLayout::get() {
    return *m_pipelineLayout;
}

PipelineLayout::PipelineLayout(vk::Device& logicalDevice, vk::DescriptorSetLayout setLayout) :
    m_logicalDevice(logicalDevice) {

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &setLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    m_pipelineLayout = logicalDevice.createPipelineLayoutUnique(pipelineLayoutInfo);
}