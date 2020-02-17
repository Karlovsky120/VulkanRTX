#include "PipelineLayout.h"

#include "LogicalDevice.h"

vk::PipelineLayout& PipelineLayout::get() {
    return m_pipelineLayout;
}

PipelineLayout::PipelineLayout(LogicalDevice& logicalDevice) :
    m_logicalDevice(logicalDevice) {

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    m_pipelineLayout = logicalDevice.get().createPipelineLayout(pipelineLayoutInfo);
}

PipelineLayout::~PipelineLayout() {
    m_logicalDevice.get().destroyPipelineLayout(m_pipelineLayout);
}