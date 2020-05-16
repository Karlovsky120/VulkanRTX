#pragma once

#include "VulkanInclude.h"

#include <map>
#include <vector>

class DeviceMemory;
class AllocId;

class MemoryAllocator {
public:
	static void init(
		std::shared_ptr<MemoryAllocator>& storage,
		const vk::Device& logicalDevice,
		const vk::PhysicalDeviceMemoryProperties& m_memoryProperties);

	static std::shared_ptr<MemoryAllocator> get();

	std::unique_ptr<AllocId> allocate(
		vk::MemoryRequirements& requirements,
		vk::MemoryPropertyFlags memoryFlags,
		vk::MemoryAllocateFlags allocateFlags = vk::MemoryAllocateFlags());

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

	std::map<std::pair<uint32_t, vk::MemoryAllocateFlags>, std::vector<DeviceMemory>> m_memoryTable;
	const vk::PhysicalDeviceMemoryProperties& m_memoryProperties;
	const vk::Device& m_logicalDevice;

	static std::weak_ptr<MemoryAllocator> instance;
};

class AllocId {
public:
	vk::DeviceMemory* memory;
	vk::MemoryAllocateFlags allocateFlags;
	uint32_t type;
	uint32_t chunk;
	uint32_t offset;

	AllocId(AllocId const&) = delete;
	void operator=(AllocId const&) = delete;

	AllocId(
		vk::DeviceMemory* memory,
		uint32_t type,
		vk::MemoryAllocateFlags flags,
		uint32_t chunk,
		uint32_t offset) :

		memory(memory),
		type(type),
		allocateFlags(flags),
		chunk(chunk),
		offset(offset) {}

	~AllocId() {
		MemoryAllocator::get()->free(*this);
	}
};