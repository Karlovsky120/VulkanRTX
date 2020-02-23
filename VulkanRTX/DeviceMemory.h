#pragma once

#include <vulkan/vulkan.hpp>

#include <list>

struct MemoryChunk {
	uint32_t offset;
	uint32_t size;
	bool free;
};

class DeviceMemory {
public:
	vk::DeviceMemory& get();

	DeviceMemory(vk::Device& logicalDevice, const uint32_t memoryType);

	std::pair<vk::DeviceMemory*, uint32_t> allocateBlock(uint32_t size, uint32_t alignment);
	void freeBlock(uint32_t offset);

private:
	//This is managed by a static class,
	//it needs to be explicitly released before the vk::Device,
	//hence no Unique handle.
	vk::DeviceMemory m_deviceMemory;

	// 256MB
	const uint32_t m_size = 268435456;

	std::list<MemoryChunk> m_blocks;

	vk::Device& m_logicalDevice;
};

