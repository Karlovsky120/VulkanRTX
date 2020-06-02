#include "Buffer.h"

vk::Buffer& Buffer::get() {
	return *m_buffer;
}

const vk::MemoryRequirements& Buffer::getMemoryRequirements() const {
	return m_memoryRequirements;
}

Buffer::Buffer(
	const vk::DeviceSize size,
	const vk::BufferUsageFlags usageFlags,
	const vk::MemoryPropertyFlags memoryFlags,
	const std::string name,
	vk::MemoryAllocateFlags memoryAllocateFlags,
	const bool dedicatedAlloc) :

	m_memoryFlags(memoryFlags),
	m_name(name) {

	vk::BufferCreateInfo bufferInfo;
	bufferInfo.size = size;
	bufferInfo.usage = usageFlags;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	m_buffer = VulkanContext::getDevice().createBufferUnique(bufferInfo);

	NAME_OBJECT(&*m_buffer,
		vk::DebugReportObjectTypeEXT::eBuffer,
		m_name)

	m_memoryRequirements = VulkanContext::getDevice().getBufferMemoryRequirements(*m_buffer);

	if (usageFlags & vk::BufferUsageFlagBits::eShaderDeviceAddress) {
		memoryAllocateFlags |= vk::MemoryAllocateFlagBits::eDeviceAddress;
	}

	m_allocId = MemoryAllocator::allocate(
		m_memoryRequirements,
		m_memoryFlags,
		memoryAllocateFlags);

	VulkanContext::getDevice().bindBufferMemory(
		*m_buffer,
		m_allocId->memory(),
		m_allocId->offset);
}