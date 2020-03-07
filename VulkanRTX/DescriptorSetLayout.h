#pragma once

#include <glm/mat4x4.hpp>
#include <vulkan/vulkan.hpp>

struct UniformBufferObject {
	glm::mat4 model;
};

class DescriptorSetLayout {
public:
	vk::DescriptorSetLayout& get();

	DescriptorSetLayout(vk::Device& logicalDevice);

private:
	vk::UniqueDescriptorSetLayout m_setLayout;
};

