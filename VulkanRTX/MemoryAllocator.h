#pragma once

#include <vulkan/vulkan.hpp>

#include <map>
#include <vector>

class DeviceMemory;
class PhysicalDevice;

struct AllocId {
	vk::DeviceMemory* memory;
	uint32_t type;
	uint32_t chunk;
	uint32_t offset;
};

class MemoryAllocator {
public:
	static void init(vk::PhysicalDevice* physicalDevice, vk::Device* logicalDevice);
	static void deinit();

	static AllocId allocate(vk::MemoryRequirements& requirements, vk::MemoryPropertyFlags memoryFlags);
	static void free(AllocId& allocId);

private:
	static bool initialized;

	static vk::PhysicalDeviceMemoryProperties m_memoryProperties;

	static uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags propertyFlags);

	static std::map<uint32_t, std::vector<DeviceMemory>> m_memoryTable;

	static vk::Device* m_logicalDevice;
};

