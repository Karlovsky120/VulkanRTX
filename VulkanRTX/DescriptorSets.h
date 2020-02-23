#pragma once

#include <vulkan/vulkan.hpp>

class DescriptorSets {
public:
	vk::DescriptorSet& get(uint32_t index);

	DescriptorSets(vk::Device& logicalDevice,
				   vk::DescriptorPool& descriptorPool,
				   std::vector<vk::DescriptorSetLayout> setLayouts);

private:
	std::vector<vk::UniqueDescriptorSet> m_descriptorSets;
};

