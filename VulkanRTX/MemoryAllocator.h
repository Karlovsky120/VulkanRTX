#pragma once

#include "GlobalDefines.h"

#include "VulkanInclude.h"

#include <map>
#include <vector>

class AllocId;
class DeviceMemory;

// 256MB
#define BLOCK_SIZE 268435456

class MemoryAllocator {
public:
	static void init(
		std::shared_ptr<MemoryAllocator>& storage,
		const vk::PhysicalDeviceMemoryProperties& m_memoryProperties);

	static std::unique_ptr<AllocId> allocate(
		vk::MemoryRequirements& requirements,
		vk::MemoryPropertyFlags memoryFlags,
		vk::MemoryAllocateFlags allocateFlags = vk::MemoryAllocateFlags(),
		const bool dedicated = false);

	static void free(AllocId& allocId);
	static vk::DeviceMemory& getMemory(AllocId& allocId);

	void freeAllMemory();

	MemoryAllocator(MemoryAllocator const&) = delete;
	void operator=(MemoryAllocator const&) = delete;

	MemoryAllocator(
		const vk::PhysicalDeviceMemoryProperties& memoryProperties);

	~MemoryAllocator();

private:
	uint32_t findMemoryType(
		uint32_t typeFilter,
		vk::MemoryPropertyFlags propertyFlags);

	std::map<std::pair<uint32_t, vk::MemoryAllocateFlags>, std::vector<DeviceMemory>> m_memoryTable;
	const vk::PhysicalDeviceMemoryProperties& m_memoryProperties;

	static std::weak_ptr<MemoryAllocator> instance;
};

class AllocId {
public:
	uint32_t type;
	vk::MemoryAllocateFlags allocateFlags;
	uint32_t chunk;
	uint32_t offset;

	AllocId(AllocId const&) = delete;
	void operator=(AllocId const&) = delete;

	AllocId(
		uint32_t type,
		vk::MemoryAllocateFlags flags,
		uint32_t chunk,
		uint32_t offset) :

		type(type),
		allocateFlags(flags),
		chunk(chunk),
		offset(offset) {}

	vk::DeviceMemory& memory() {
		return MemoryAllocator::getMemory(*this);
	}

	~AllocId() {
		MemoryAllocator::free(*this);
	}
};