#include "Buffer.h"

#include "MemoryAllocator.h"

vk::Buffer& Buffer::get() {
	return *m_buffer;
}

vk::Buffer* Buffer::getPtr() {
	return &*m_buffer;
}

const vk::MemoryRequirements& Buffer::getMemoryRequirements() const {
	return m_memoryRequirements;
}

Buffer::Buffer(
	vk::Device& logicalDevice,
	const vk::DeviceSize size,
	const vk::BufferUsageFlags usageFlags,
	const vk::MemoryPropertyFlags memoryFlags,
	const vk::MemoryAllocateFlags memoryAllocateFlags,
	const vk::MemoryRequirements memoryRequirments) :

	m_logicalDevice(logicalDevice),
	m_memoryFlags(memoryFlags),
	m_memoryRequirements(memoryRequirments) {

	vk::BufferCreateInfo bufferInfo;
	bufferInfo.size = size;
	bufferInfo.usage = usageFlags;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	m_buffer = m_logicalDevice.createBufferUnique(bufferInfo);

	if (memoryRequirments.size == 0) {
		m_memoryRequirements = m_logicalDevice.getBufferMemoryRequirements(*m_buffer);
	}

	m_allocId = MemoryAllocator::get()->allocate(m_memoryRequirements, m_memoryFlags, memoryAllocateFlags);
	m_logicalDevice.bindBufferMemory(*m_buffer, *m_allocId.memory, m_allocId.offset);
}

Buffer::~Buffer() {
	MemoryAllocator::get()->free(m_allocId);
}