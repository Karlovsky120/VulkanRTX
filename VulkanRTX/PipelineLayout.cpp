#include "PipelineLayout.h"

#include <vector>

vk::PipelineLayout& PipelineLayout::get() {
    return *m_pipelineLayout;
}

PipelineLayout::PipelineLayout(vk::Device& logicalDevice, vk::DescriptorSetLayout setLayout) :
    m_logicalDevice(logicalDevice) {

    std::vector<vk::PushConstantRange> pushConstantRanges;

    vk::PushConstantRange cameraMatrix;
    cameraMatrix.stageFlags = vk::ShaderStageFlagBits::eVertex;
    cameraMatrix.offset = 0;
    cameraMatrix.size = 16 * sizeof(float);

    pushConstantRanges.push_back(cameraMatrix);

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &setLayout;
    pipelineLayoutInfo.pushConstantRangeCount = pushConstantRanges.size();
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

    m_pipelineLayout = logicalDevice.createPipelineLayoutUnique(pipelineLayoutInfo);
}