#pragma once

#include "VulkanInclude.h"

#include "CommandBuffer.h"
#include "MemoryAllocator.h"

class Buffer {
public:
	std::unique_ptr<AllocId> m_allocId;

	vk::Buffer& get();

	const vk::MemoryRequirements& getMemoryRequirements() const;

	template <class T>
	void uploadToBuffer(T data) {
		std::vector<T> dataVector = { data };
		uploadToBuffer(dataVector);
	}

	template <class T>
	void uploadToBuffer(std::vector<T>& data) {
		if (m_memoryFlags & vk::MemoryPropertyFlagBits::eHostVisible) {
			uploadToHostVisible(data);
		}
		else if (m_memoryFlags & vk::MemoryPropertyFlagBits::eDeviceLocal) {
			uploadToDeviceLocal(data);
		}
		else {
			throw std::runtime_error("Unhandled memory property on buffer copy!");
		}
	}

	Buffer(
		vk::Device& logicalDevice,
		const vk::DeviceSize size,
		const vk::BufferUsageFlags usageFlags,
		const vk::MemoryPropertyFlags memoryFlags,
		vk::MemoryAllocateFlags memoryAllocateFlags = vk::MemoryAllocateFlags());

private:
	template <class T>
	void uploadToHostVisible(std::vector<T>& data) {
		uint32_t sizeInBytes = static_cast<uint32_t>(data.size() * sizeof(T));

		void* bufferData = m_logicalDevice.mapMemory(*m_allocId->memory,
													 m_allocId->offset,
													 sizeInBytes,
													 vk::MemoryMapFlags());

		memcpy(bufferData, data.data(), sizeInBytes);
		m_logicalDevice.unmapMemory(*m_allocId->memory);
	}

	template <class T>
	void uploadToDeviceLocal(std::vector<T>& data) {
		uint32_t sizeInBytes = static_cast<uint32_t>(data.size() * sizeof(T));

		Buffer hostLocal = Buffer(
			m_logicalDevice,
			sizeInBytes,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible);

		hostLocal.uploadToBuffer(data);

		vk::BufferCopy bufferCopy;
		bufferCopy.size = sizeInBytes;
		bufferCopy.srcOffset = 0;
		bufferCopy.dstOffset = 0;

		CommandBuffer transferCmdBuffer = CommandBuffer(PoolType::eTransfer);
		transferCmdBuffer.get().copyBuffer(hostLocal.get(), *m_buffer, bufferCopy);
		transferCmdBuffer.submit(true);
	}

	vk::UniqueBuffer m_buffer;

	vk::MemoryPropertyFlags m_memoryFlags;
	vk::MemoryRequirements m_memoryRequirements;

	vk::Device& m_logicalDevice;
};

