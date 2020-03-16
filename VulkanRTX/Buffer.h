#pragma once

#include "VulkanInclude.h"

#include "CmdBufferAllocator.h"
#include "MemoryAllocator.h"

class Buffer {
public:
	AllocId m_allocId;

	vk::Buffer& get();
	vk::Buffer* getPtr();

	const vk::MemoryRequirements& getMemoryRequirements() const;

	template <class T>
	void uploadToHostLocal(T data) {
		uploadToHostLocal(std::vector<T>{data});
	}

	template <class T>
	void uploadToHostLocal(std::vector<T> data) {
		assert(m_memoryFlags & vk::MemoryPropertyFlagBits::eHostVisible);

		uint32_t sizeInBytes = static_cast<uint32_t>(data.size() * sizeof(T));

		void* bufferData = m_logicalDevice.mapMemory(*m_allocId.memory,
													 m_allocId.offset,
													 sizeInBytes,
													 vk::MemoryMapFlags());

		memcpy(bufferData, data.data(), sizeInBytes);
		m_logicalDevice.unmapMemory(*m_allocId.memory);
	}

	template <class T>
	void uploadToDeviceLocal(std::vector<T>& data) {

		assert(m_memoryFlags & vk::MemoryPropertyFlagBits::eDeviceLocal);

		uint32_t dataSize = static_cast<uint32_t>(sizeof(T) * data.size());

		Buffer hostLocal = Buffer(
			m_logicalDevice,
			dataSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible);

		hostLocal.uploadToHostLocal(data);

		vk::BufferCopy bufferCopy;
		bufferCopy.size = dataSize;
		bufferCopy.srcOffset = 0;
		bufferCopy.dstOffset = 0;

		vk::UniqueCommandBuffer transferCmdBuffer = CmdBufferAllocator::get()->getTransferBuffer();

		vk::CommandBufferBeginInfo beginInfo;
		transferCmdBuffer->begin(beginInfo);
		transferCmdBuffer->copyBuffer(hostLocal.get(), *m_buffer, bufferCopy);
		transferCmdBuffer->end();

		CmdBufferAllocator::get()->submitBuffer(*transferCmdBuffer, true);
	}

	Buffer(vk::Device& logicalDevice,
		   const vk::DeviceSize size,
		   const vk::BufferUsageFlags usageFlags,
		   const vk::MemoryPropertyFlags memoryFlags);

	~Buffer();

private:
	vk::UniqueBuffer m_buffer;

	vk::MemoryPropertyFlags m_memoryFlags;
	vk::MemoryRequirements m_memoryRequirements;

	vk::Device& m_logicalDevice;
};

