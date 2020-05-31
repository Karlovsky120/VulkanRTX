#include "RayTracing.h"
#include "Buffer.h"
#include "CommandBuffer.h"
#include "Image.h"
#include "Mesh.h"
#include "Vertex.h"

#include <fstream>
#include <vector>


std::unique_ptr<Buffer> RayTracing::createSBTable(vk::Pipeline& pipeline) {
	uint32_t sbTableSize = VulkanContext::get()->m_rayTracingProperties.shaderGroupHandleSize * 3;

	std::unique_ptr<Buffer> sbTable = std::make_unique<Buffer>(
		sbTableSize,
		vk::BufferUsageFlagBits::eTransferDst
		| vk::BufferUsageFlagBits::eRayTracingKHR
		| vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		"SBTable");

	std::vector<uint8_t> handleData(sbTableSize);
	VulkanContext::getDevice().getRayTracingShaderGroupHandlesKHR(pipeline, 0, 3, sbTableSize, handleData.data());

	sbTable->uploadToBuffer(handleData);
	return std::move(sbTable);
}

vk::UniqueDescriptorSetLayout RayTracing::createDescriptorSetLayout() {
	vk::DescriptorSetLayoutBinding accelerationStructureBinding;
	accelerationStructureBinding.binding = 0;
	accelerationStructureBinding.descriptorType = vk::DescriptorType::eAccelerationStructureKHR;
	accelerationStructureBinding.descriptorCount = 1;
	accelerationStructureBinding.stageFlags = vk::ShaderStageFlagBits::eRaygenKHR;

	vk::DescriptorSetLayoutBinding resultImageBinding;
	resultImageBinding.binding = 1;
	resultImageBinding.descriptorType = vk::DescriptorType::eStorageImage;
	resultImageBinding.descriptorCount = 1;
	resultImageBinding.stageFlags = vk::ShaderStageFlagBits::eRaygenKHR;

	vk::DescriptorSetLayoutBinding uniformBufferBinding;
	uniformBufferBinding.binding = 2;
	uniformBufferBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
	uniformBufferBinding.descriptorCount = 1;
	uniformBufferBinding.stageFlags = vk::ShaderStageFlagBits::eRaygenKHR;

	vk::DescriptorSetLayoutBinding chunkVerticesBufferBinding;
	chunkVerticesBufferBinding.binding = 3;
	chunkVerticesBufferBinding.descriptorType = vk::DescriptorType::eStorageBuffer;
	chunkVerticesBufferBinding.descriptorCount = 1;
	chunkVerticesBufferBinding.stageFlags = vk::ShaderStageFlagBits::eClosestHitKHR;

	vk::DescriptorSetLayoutBinding chunkIndicesBufferBinding;
	chunkIndicesBufferBinding.binding = 4;
	chunkIndicesBufferBinding.descriptorType = vk::DescriptorType::eStorageBuffer;
	chunkIndicesBufferBinding.descriptorCount = 1;
	chunkIndicesBufferBinding.stageFlags = vk::ShaderStageFlagBits::eClosestHitKHR;

	std::vector<vk::DescriptorSetLayoutBinding> bindings = {
		accelerationStructureBinding,
		resultImageBinding,
		uniformBufferBinding,
		chunkVerticesBufferBinding,
		chunkIndicesBufferBinding
	};

	vk::DescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	return VulkanContext::getDevice().createDescriptorSetLayoutUnique(layoutInfo);
}

vk::UniqueDescriptorPool RayTracing::createDescriptorPool() {
	std::vector<vk::DescriptorPoolSize> poolSizes = {
		{vk::DescriptorType::eAccelerationStructureKHR, 1},
		{vk::DescriptorType::eStorageImage, 1},
		{vk::DescriptorType::eUniformBuffer, 1}
	};

	vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo;
	descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();
	descriptorPoolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
	descriptorPoolCreateInfo.maxSets = 1;
	return VulkanContext::getDevice().createDescriptorPoolUnique(descriptorPoolCreateInfo);
}

vk::UniqueDescriptorSet RayTracing::createDescriptorSet(
	vk::DescriptorPool& descriptorPool,
	vk::DescriptorSetLayout& descriptorSetLayout) {

	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	return std::move(VulkanContext::getDevice().allocateDescriptorSetsUnique(descriptorSetAllocateInfo)[0]);
}