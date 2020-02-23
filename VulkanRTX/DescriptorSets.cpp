#include "DescriptorSets.h"

vk::DescriptorSet& DescriptorSets::get(uint32_t index) {
	return *m_descriptorSets[index];
}


DescriptorSets::DescriptorSets(vk::Device& logicalDevice,
							   vk::DescriptorPool& descriptorPool,
							   std::vector<vk::DescriptorSetLayout> setLayouts) {

	uint32_t count = setLayouts.size();
	m_descriptorSets.resize(count);

	vk::DescriptorSetAllocateInfo allocInfo;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = count;
	allocInfo.pSetLayouts = setLayouts.data();

	m_descriptorSets = logicalDevice.allocateDescriptorSetsUnique(allocInfo);
}