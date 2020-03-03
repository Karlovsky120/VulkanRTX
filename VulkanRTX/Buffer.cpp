#include "Buffer.h"

#include "LogicalDevice.h"
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

Buffer::Buffer(vk::Device& logicalDevice, const uint32_t size, const vk::BufferUsageFlags usageFlags, const vk::MemoryPropertyFlags memoryFlags) :
	m_logicalDevice(logicalDevice),
	m_memoryFlags(memoryFlags) {

	vk::BufferCreateInfo bufferInfo;
	bufferInfo.size = vk::DeviceSize(size);
	bufferInfo.usage = usageFlags;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	m_buffer = logicalDevice.createBufferUnique(bufferInfo);

	m_memoryRequirements = logicalDevice.getBufferMemoryRequirements(*m_buffer);

	m_allocId = MemoryAllocator::allocate(m_memoryRequirements, memoryFlags);

	logicalDevice.bindBufferMemory(*m_buffer, *m_allocId.memory, m_allocId.offset);
}

void Buffer::copyBuffersToGPU(vk::CommandBuffer& commandBuffer,
						 std::vector<vk::Buffer*>& src,
						 std::vector<vk::Buffer*>& dst,
						 std::vector<vk::BufferCopy>& bufferCopies) {

	for (int i = 0; i < bufferCopies.size(); ++i) {
		commandBuffer.copyBuffer(*src[i], *dst[i], bufferCopies[i]);
	}
}