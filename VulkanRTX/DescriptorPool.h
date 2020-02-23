#pragma once

#include <vulkan/vulkan.hpp>

class DescriptorPool {
public:
	vk::DescriptorPool& get();

	DescriptorPool(vk::Device& logicalDevice, uint32_t descriptorCount);

private:
	vk::UniqueDescriptorPool m_descriptorPool;

};

