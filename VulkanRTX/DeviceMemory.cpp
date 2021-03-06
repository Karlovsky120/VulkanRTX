#include "DeviceMemory.h"

#include "VulkanContext.h"

#include <exception>

vk::DeviceMemory& DeviceMemory::get() {
	return m_deviceMemory;
}

// Find first free block with enough space when adjusted for alignment.
// Any space lost at the start of the block due to alignment is added to the previous block
uint32_t DeviceMemory::allocateBlock(
	size_t requestedSize,
	size_t alignment) {

	for (auto currentBlock = m_blocks.begin(); currentBlock != m_blocks.end(); ++currentBlock) {
		uint32_t alignmentShift = (alignment - currentBlock->offset) & (alignment - 1);
		uint32_t alignedSize = currentBlock->size - alignmentShift;

		if (currentBlock->free && requestedSize <= alignedSize) {
			currentBlock->offset += alignmentShift;
			currentBlock->size = alignedSize;

			if (currentBlock != m_blocks.begin()) {
				std::prev(currentBlock)->size += alignmentShift;
			}

			if (currentBlock->size != requestedSize) {
				uint32_t newBlockSize = currentBlock->size - requestedSize;
				m_blocks.insert(std::next(currentBlock), MemoryChunk{currentBlock->offset + requestedSize, newBlockSize, true});
				currentBlock->size = requestedSize;
			}
			
			currentBlock->free = false;

			return currentBlock->offset;
		}
	}

	return -1;
}

bool DeviceMemory::freeBlock(uint32_t freeOffset) {
	for (auto currentBlock = m_blocks.begin(); currentBlock != m_blocks.end(); ++currentBlock) {
		if (currentBlock->offset == freeOffset) {
			currentBlock->free = true;

			if (currentBlock != m_blocks.begin()) {
				auto previousBlock = std::prev(currentBlock);
				if (previousBlock->free) {
					currentBlock->size += previousBlock->size;
					currentBlock->offset = previousBlock->offset;
					m_blocks.erase(previousBlock);
				}
			}

			auto nextBlock = std::next(currentBlock);
			if (nextBlock != m_blocks.end() && nextBlock->free) {
				currentBlock->size += nextBlock->size;
				m_blocks.erase(nextBlock);
			}

			break;
		}
	}

	/*if (m_blocks.size() == 1) {
		VulkanContext::getDevice().freeMemory(m_deviceMemory);
		return true;
	}*/

	return false;
}

DeviceMemory::DeviceMemory(
	const uint32_t size,
	const uint32_t memoryTypeIndex,
	const vk::MemoryAllocateFlags flags) :

	m_blocks(std::list<MemoryChunk>()) {

	m_blocks.insert(m_blocks.begin(), MemoryChunk{0, size, true});

	void* currentFeature = nullptr;

	vk::MemoryAllocateFlagsInfo flagInfo;
	if (flags != vk::MemoryAllocateFlags()) {
		flagInfo.flags = flags;
		currentFeature = &flagInfo;
	}

#ifdef OPTIX_DENOISER
	vk::ExportMemoryAllocateInfo exportInfo;
	exportInfo.handleTypes = vk::ExternalMemoryHandleTypeFlagBits::eOpaqueWin32;
	exportInfo.pNext = currentFeature;
	currentFeature = &exportInfo;
#endif

	vk::MemoryAllocateInfo allocateInfo;
	allocateInfo.allocationSize = size;
	allocateInfo.memoryTypeIndex = memoryTypeIndex;
	allocateInfo.pNext = currentFeature;

	m_deviceMemory = VulkanContext::getDevice().allocateMemory(allocateInfo);
}
