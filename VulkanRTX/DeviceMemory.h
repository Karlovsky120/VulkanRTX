#pragma once

#include "GlobalDefines.h"

#include "VulkanInclude.h"

#include <list>

struct MemoryChunk {
	size_t offset;
	size_t size;
	bool free;
};

class DeviceMemory {
public:
	vk::DeviceMemory& get();

	DeviceMemory(
		const vk::Device& logicalDevice,
		const uint32_t memoryTypeIndex,
		const vk::MemoryAllocateFlags flags);

	std::pair<vk::DeviceMemory*, uint32_t> allocateBlock(
		size_t size,
		size_t alignment);

	void freeBlock(uint32_t offset);

private:
	//This is managed by MemoryAllocator,
	//it needs to be explicitly released before the vk::Device,
	//hence no Unique handle.
	vk::DeviceMemory m_deviceMemory;

	// 256MB
	const uint32_t m_size = 268435456;

	std::list<MemoryChunk> m_blocks;
};

