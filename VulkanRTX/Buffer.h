#pragma once

#include "VulkanInclude.h"

#include "MemoryAllocator.h"

class Buffer {
public:
	vk::Buffer& get();
	vk::Buffer* getPtr();

	const vk::MemoryRequirements& getMemoryRequirements() const;

	template <class T>
	void copyToBuffer(T data) {
		copyToBuffer(std::vector<T>{data});
	}

	template <class T>
	void copyToBuffer(std::vector<T> data) {
		assert(m_memoryFlags & vk::MemoryPropertyFlagBits::eHostVisible);

		uint32_t sizeInBytes = static_cast<uint32_t>(data.size() * sizeof(T));

		void* bufferData = m_logicalDevice.mapMemory(*m_allocId.memory,
													 m_allocId.offset,
													 sizeInBytes,
													 vk::MemoryMapFlags());

		memcpy(bufferData, data.data(), sizeInBytes);
		m_logicalDevice.unmapMemory(*m_allocId.memory);
	}

	static void copyBuffersToGPU(vk::CommandBuffer& commandBuffer,
							std::vector<vk::Buffer*>& src,
							std::vector<vk::Buffer*>& dst,
							std::vector<vk::BufferCopy>& bufferCopies);

	Buffer(vk::Device& logicalDevice,
		   const vk::DeviceSize size,
		   const vk::BufferUsageFlags usageFlags,
		   const vk::MemoryPropertyFlags memoryFlags);

	~Buffer();

private:
	vk::UniqueBuffer m_buffer;
	AllocId m_allocId;

	vk::MemoryPropertyFlags m_memoryFlags;
	vk::MemoryRequirements m_memoryRequirements;

	vk::Device& m_logicalDevice;
};

