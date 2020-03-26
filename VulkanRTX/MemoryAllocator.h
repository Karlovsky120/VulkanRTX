#pragma once

#include "VulkanInclude.h"

#include <map>
#include <vector>

class DeviceMemory;

struct AllocId {
	vk::DeviceMemory* memory;
	uint32_t type;
	uint32_t chunk;
	uint32_t offset;
};

class MemoryAllocator {
public:
	static void init(
		std::shared_ptr<MemoryAllocator>& storage,
		const vk::Device& logicalDevice,
		const vk::PhysicalDeviceMemoryProperties& m_memoryProperties);

	static std::shared_ptr<MemoryAllocator> get();

	AllocId allocate(
		vk::MemoryRequirements& requirements,
		vk::MemoryPropertyFlags memoryFlags,
		vk::MemoryAllocateFlags allocateFlags);

	void free(AllocId& allocId);
	void freeAllMemory();

	MemoryAllocator(MemoryAllocator const&) = delete;
	void operator=(MemoryAllocator const&) = delete;

	MemoryAllocator(
		const vk::Device& logicalDevice,
		const vk::PhysicalDeviceMemoryProperties& memoryProperties);
	~MemoryAllocator();

private:
	uint32_t findMemoryType(
		uint32_t typeFilter,
		vk::MemoryPropertyFlags propertyFlags);

	std::map<uint32_t, std::vector<DeviceMemory>> m_memoryTable;
	const vk::PhysicalDeviceMemoryProperties& m_memoryProperties;
	const vk::Device& m_logicalDevice;

	static std::weak_ptr<MemoryAllocator> instance;
};

