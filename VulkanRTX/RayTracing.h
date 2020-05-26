#pragma once

#include "GlobalDefines.h"

#include "MemoryAllocator.h"
#include "VulkanContext.h"

#include "VulkanInclude.h"

#include <glm/mat4x4.hpp>
#include <string>

class Buffer;
class Image;
class Mesh;

class RayTracing {
public:
	std::unique_ptr<Image> m_storageImage;

	std::unique_ptr<Buffer> createSBTable(vk::Pipeline& pipeline);
	std::unique_ptr<Image> createStorageImage(uint32_t width, uint32_t height);

	vk::UniqueDescriptorPool createDescriptorPool();
	vk::UniqueDescriptorSetLayout createDescriptorSetLayout();
	vk::UniqueDescriptorSet createDescriptorSet(
		vk::DescriptorPool& descriptorPool, 
		vk::DescriptorSetLayout& descriptorSetLayout);
};

