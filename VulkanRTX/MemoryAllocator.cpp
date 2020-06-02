#include "MemoryAllocator.h"

#include "DeviceMemory.h"
#include "VulkanContext.h"

void MemoryAllocator::init(
    std::shared_ptr<MemoryAllocator>& storage,
    const vk::PhysicalDeviceMemoryProperties& memoryProperties) {

    storage = std::make_shared<MemoryAllocator>(memoryProperties);
    instance = storage;
}

std::unique_ptr<AllocId> MemoryAllocator::allocate(
    vk::MemoryRequirements& requirements,
    vk::MemoryPropertyFlags memoryFlags,
    vk::MemoryAllocateFlags allocateFlags,
    const bool dedicated) {

    uint32_t deviceMemoryType = instance.lock()->findMemoryType(requirements.memoryTypeBits, memoryFlags);
    auto memoryId = std::make_pair(deviceMemoryType, allocateFlags);

    auto& memoryTable = instance.lock()->m_memoryTable;
    auto it = memoryTable.find(memoryId);

    if (it == memoryTable.end()) {
        memoryTable.insert(std::make_pair(memoryId, std::vector<DeviceMemory>()));
        it = memoryTable.find(memoryId);
    }

    std::vector<DeviceMemory>& deviceMemories = it->second;

    uint32_t chunkIndex = 0;
    uint32_t blockIndex = -1;
    for (auto deviceMemory = deviceMemories.begin(); deviceMemory != deviceMemories.end(); ++deviceMemory) {
        if (!dedicated) {
            blockIndex = deviceMemory->allocateBlock(requirements.size, requirements.alignment);
            if (blockIndex != -1) {
                return std::make_unique<AllocId>(deviceMemoryType, allocateFlags, chunkIndex, blockIndex);
            }
        }
        ++chunkIndex;
    }
    if (dedicated) {
        deviceMemories.push_back(DeviceMemory(BLOCK_SIZE, deviceMemoryType, allocateFlags));
    }
    else {
        deviceMemories.push_back(DeviceMemory(requirements.size, deviceMemoryType, allocateFlags));
    }

    blockIndex = deviceMemories.back().allocateBlock(requirements.size, requirements.alignment);

    return std::make_unique<AllocId>(deviceMemoryType, allocateFlags, chunkIndex, blockIndex);
}

void MemoryAllocator::free(AllocId& allocId) {
    instance.lock()->m_memoryTable[std::make_pair(allocId.type, allocId.allocateFlags)][allocId.chunk].freeBlock(allocId.offset);
}

vk::DeviceMemory& MemoryAllocator::getMemory(AllocId& allocId) {
    return instance.lock()->m_memoryTable[std::make_pair(allocId.type, allocId.allocateFlags)][allocId.chunk].get();
}

void MemoryAllocator::freeAllMemory() {
    for (auto& memoryType : m_memoryTable) {
        for (DeviceMemory& memoryChunk : memoryType.second) {
            VulkanContext::getDevice().freeMemory(memoryChunk.get());
        }
    }
}

uint32_t MemoryAllocator::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags propertyFlags) {
    for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; ++i) {
        if ((typeFilter & (1 << i)) && (m_memoryProperties.memoryTypes[i].propertyFlags & propertyFlags)) {
            return i;
        }
    }

    throw std::runtime_error("No suitable memory type found!");
}

MemoryAllocator::MemoryAllocator(
    const vk::PhysicalDeviceMemoryProperties& memoryProperties) :
    m_memoryProperties(memoryProperties) {}

std::weak_ptr<MemoryAllocator> MemoryAllocator::instance;

MemoryAllocator::~MemoryAllocator() {
    freeAllMemory();
}