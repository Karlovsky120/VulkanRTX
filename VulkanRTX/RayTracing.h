#pragma once

#include "VulkanInclude.h"

#include "MemoryAllocator.h"
#include "VulkanContext.h"

#include <glm/mat4x4.hpp>
#include <string>

class Buffer;
class Image;
class Mesh;

/*#define INDEX_RAYGEN 0
#define INDEX_MISS 1
#define INDEX_CLOSEST_HIT 2*/

class RayTracing {
public:
	/*vk::UniqueAccelerationStructureKHR m_blas;
	vk::UniqueAccelerationStructureKHR m_tlas;
	vk::UniquePipeline m_pipeline;*/
	std::unique_ptr<Image> m_storageImage;

	/*vk::UniqueAccelerationStructureKHR& createBottomAccelerationStructure(Mesh& mesh);
	vk::UniqueAccelerationStructureKHR& createTopAccelerationStructure(vk::AccelerationStructureKHR& bottom);
	vk::UniquePipeline& createPipeline();*/
	std::unique_ptr<Buffer> createSBTable(vk::Pipeline& pipeline);
	std::unique_ptr<Image> createStorageImage(uint32_t width, uint32_t height);
	//void updateDescriptorSets(vk::ImageView& storageImageView);

	vk::UniqueDescriptorPool createDescriptorPool();
	vk::UniqueDescriptorSetLayout createDescriptorSetLayout();
	vk::UniqueDescriptorSet createDescriptorSet(
		vk::DescriptorPool& descriptorPool, 
		vk::DescriptorSetLayout& descriptorSetLayout);

	void buildCommandBuffers(uint32_t width, uint32_t height);

	RayTracing();

private:
	/*template <class T>
	T getBufferDeviceAddress(vk::Buffer& buffer) {
		vk::BufferDeviceAddressInfo addressInfo;
		addressInfo.buffer = buffer;

		return { m_context->m_logicalDevice->getBufferAddress(addressInfo) };
	}

	Buffer createScratchBuffer(
		vk::AccelerationStructureKHR& accelerationStructure,
		vk::AccelerationStructureMemoryRequirementsTypeKHR type
	);

	std::unique_ptr<AllocId> m_blasMemory;
	std::unique_ptr<AllocId> m_tlasMemory;

	vk::DeviceOrHostAddressKHR m_blasAddress;
	vk::DeviceOrHostAddressKHR m_tlasAddress;

	vk::UniqueShaderModule createShaderModule(
		const std::string shaderPath) const;*/

	std::unique_ptr<Buffer> m_uniformBuffer;

	/*vk::UniquePipelineCache m_pipelineCache;
	vk::UniquePipelineLayout m_pipelineLayout;*/

	std::unique_ptr<Buffer> m_stb;

	vk::UniqueDescriptorPool m_descriptorPool;
	vk::UniqueDescriptorSetLayout m_descriptorSetLayout;
	vk::UniqueDescriptorSet m_descriptorSet;

	std::shared_ptr<VulkanContext> m_context;
};

