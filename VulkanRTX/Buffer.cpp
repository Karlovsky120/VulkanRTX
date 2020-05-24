#include "Buffer.h"

#include "MemoryAllocator.h"

vk::Buffer& Buffer::get() {
	return *m_buffer;
}

const vk::MemoryRequirements& Buffer::getMemoryRequirements() const {
	return m_memoryRequirements;
}

Buffer::Buffer(
	vk::Device& logicalDevice,
	const vk::DeviceSize size,
	const vk::BufferUsageFlags usageFlags,
	const vk::MemoryPropertyFlags memoryFlags,
	vk::MemoryAllocateFlags memoryAllocateFlags) :

	m_memoryFlags(memoryFlags),
	m_logicalDevice(logicalDevice) {

	vk::BufferCreateInfo bufferInfo;
	bufferInfo.size = size;
	bufferInfo.usage = usageFlags;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	m_buffer = m_logicalDevice.createBufferUnique(bufferInfo);

	m_memoryRequirements = m_logicalDevice.getBufferMemoryRequirements(*m_buffer);

	if (usageFlags & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
		memoryAllocateFlags |= vk::MemoryAllocateFlagBits::eDeviceAddress;
	}

	m_allocId = MemoryAllocator::allocate(m_memoryRequirements, m_memoryFlags, memoryAllocateFlags);
	m_logicalDevice.bindBufferMemory(*m_buffer, *m_allocId->memory, m_allocId->offset);
}