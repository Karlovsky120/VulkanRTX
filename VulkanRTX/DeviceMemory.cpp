#include "DeviceMemory.h"

vk::DeviceMemory& DeviceMemory::get() {
	return m_deviceMemory;
}

// Find first free block with enough space when adjusted for alignment.
// Any space lost at the start of the block due to alignment is added to the previous block
std::pair<vk::DeviceMemory*, uint32_t> DeviceMemory::allocateBlock(uint32_t requestedSize, uint32_t alignment) {

	for (auto currentBlock = m_blocks.begin(); currentBlock != m_blocks.end(); ++currentBlock) {

		uint32_t alignmentShift = alignment - currentBlock->offset & (alignment - 1);
		uint32_t alignedSize = currentBlock->size - alignmentShift;

		if (currentBlock->free && requestedSize <= alignedSize) {
			currentBlock->offset += alignmentShift;
			currentBlock->size = alignedSize;

			if (currentBlock != m_blocks.begin()) {
				std::prev(currentBlock)->size += alignmentShift;
			}

			if (currentBlock->size != requestedSize) {
				uint32_t newBlockSize = currentBlock->size - requestedSize;
				m_blocks.insert(currentBlock, MemoryChunk{currentBlock->offset + requestedSize, newBlockSize, true});
			}
			
			currentBlock->size = requestedSize;
			currentBlock->free = false;

			return std::pair(&m_deviceMemory, currentBlock->offset);
		}
	}

	return std::pair(&m_deviceMemory, -1);
}

void DeviceMemory::freeBlock(uint32_t freeOffset) {
	for (auto currentBlock = m_blocks.begin(); currentBlock != m_blocks.end(); ++currentBlock) {
		if (currentBlock->offset = freeOffset) {
			auto previousBlock = std::prev(currentBlock);
			if (previousBlock->free) {
				currentBlock->size += previousBlock->size;
				currentBlock->offset = previousBlock->offset;
				m_blocks.erase(previousBlock);
			}

			auto nextBlock = std::next(currentBlock);
			if (nextBlock->free) {
				currentBlock->size += nextBlock->size;
				m_blocks.erase(nextBlock);
			}

			return;
		}
	}
}

DeviceMemory::DeviceMemory(vk::Device& logicalDevice, const uint32_t memoryType) :
	m_logicalDevice(logicalDevice),
	m_blocks(std::list<MemoryChunk>()) {

	m_blocks.insert(m_blocks.begin(), MemoryChunk{0, m_size, true});

	vk::MemoryAllocateInfo allocateInfo;
	allocateInfo.allocationSize = m_size;
	allocateInfo.memoryTypeIndex = memoryType;

	m_deviceMemory = logicalDevice.allocateMemory(allocateInfo);
}