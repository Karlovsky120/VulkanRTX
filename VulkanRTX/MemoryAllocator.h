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
		vk::Device& logicalDevice,
		const vk::PhysicalDeviceMemoryProperties& m_memoryProperties);
	static MemoryAllocator* getMemoryAllocator();

	AllocId allocate(vk::MemoryRequirements& requirements,
		vk::MemoryPropertyFlags memoryFlags);

	void free(AllocId& allocId);
	void freeAllMemory();

	static void destroy();

	MemoryAllocator(MemoryAllocator const&) = delete;
	void operator=(MemoryAllocator const&) = delete;

private:
	MemoryAllocator(vk::Device& logicalDevice,
		const vk::PhysicalDeviceMemoryProperties& memoryProperties);

	uint32_t findMemoryType(uint32_t typeFilter,
		vk::MemoryPropertyFlags propertyFlags);

	std::map<uint32_t, std::vector<DeviceMemory>> m_memoryTable;
	const vk::PhysicalDeviceMemoryProperties& m_memoryProperties;
	const vk::Device& m_logicalDevice;

	static MemoryAllocator* instance;
};

