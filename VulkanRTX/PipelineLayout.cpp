#include "PipelineLayout.h"

vk::PipelineLayout& PipelineLayout::get() {
    return *m_pipelineLayout;
}

PipelineLayout::PipelineLayout(vk::Device& logicalDevice) :
    m_logicalDevice(logicalDevice) {

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    m_pipelineLayout = logicalDevice.createPipelineLayoutUnique(pipelineLayoutInfo);
}