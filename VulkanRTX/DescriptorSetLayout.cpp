#include "DescriptorSetLayout.h"

vk::DescriptorSetLayout& DescriptorSetLayout::get() {
	return *m_setLayout;
}

DescriptorSetLayout::DescriptorSetLayout(vk::Device& logicalDevice) {
	vk::DescriptorSetLayoutBinding uboLayoutBinding;
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	vk::DescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	m_setLayout = logicalDevice.createDescriptorSetLayoutUnique(layoutInfo);
}