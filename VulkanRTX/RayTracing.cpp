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

	// I would very much like this to be DeviceLocal, but the deriver crashes for some reason if I try.
	std::unique_ptr<Buffer> sbTable = std::make_unique<Buffer>(
		*VulkanContext::get()->m_logicalDevice,
		sbTableSize,
		vk::BufferUsageFlagBits::eTransferDst
		| vk::BufferUsageFlagBits::eRayTracingKHR
		| vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		"SBTable");

	std::vector<uint8_t> handleData(sbTableSize);
	VulkanContext::get()->m_logicalDevice->getRayTracingShaderGroupHandlesKHR(pipeline, 0, 3, sbTableSize, handleData.data());

	sbTable->uploadToBuffer(handleData);
	return std::move(sbTable);
}

std::unique_ptr<Image> RayTracing::createStorageImage(uint32_t width, uint32_t height) {
	return std::make_unique<Image>(
		*VulkanContext::get()->m_logicalDevice,
		width,
		height,
		vk::Format::eB8G8R8A8Unorm,
		"RTX render target",
		vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage,
		vk::ImageAspectFlagBits::eColor,
		vk::ImageLayout::eGeneral);
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

	std::vector<vk::DescriptorSetLayoutBinding> bindings = {
		accelerationStructureBinding,
		resultImageBinding,
		uniformBufferBinding
	};

	vk::DescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	return VulkanContext::get()->m_logicalDevice->createDescriptorSetLayoutUnique(layoutInfo);
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
	return VulkanContext::get()->m_logicalDevice->createDescriptorPoolUnique(descriptorPoolCreateInfo);
}

vk::UniqueDescriptorSet RayTracing::createDescriptorSet(
	vk::DescriptorPool& descriptorPool,
	vk::DescriptorSetLayout& descriptorSetLayout) {

	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo;
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	return std::move(VulkanContext::get()->m_logicalDevice->allocateDescriptorSetsUnique(descriptorSetAllocateInfo)[0]);
}