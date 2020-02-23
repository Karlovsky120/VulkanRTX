#include "DescriptorPool.h"

vk::DescriptorPool& DescriptorPool::get() {
	return *m_descriptorPool;
}

DescriptorPool::DescriptorPool(vk::Device& logicalDevice, uint32_t descriptorSetCount) {
	vk::DescriptorPoolSize poolSize;
	poolSize.descriptorCount = descriptorSetCount;

	vk::DescriptorPoolCreateInfo poolInfo;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = descriptorSetCount;

	m_descriptorPool = logicalDevice.createDescriptorPoolUnique(poolInfo);
}