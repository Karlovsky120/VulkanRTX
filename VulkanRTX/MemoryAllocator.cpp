#include "MemoryAllocator.h"

#include "DeviceMemory.h"

void MemoryAllocator::init(
    vk::Device& logicalDevice,
    const vk::PhysicalDeviceMemoryProperties& memoryProperties) {

    instance = new MemoryAllocator(logicalDevice, memoryProperties);
}

MemoryAllocator* MemoryAllocator::getMemoryAllocator() {
    return instance;
}

AllocId MemoryAllocator::allocate(vk::MemoryRequirements& requirements, vk::MemoryPropertyFlags memoryFlags) {
    uint32_t hostMemoryType = findMemoryType(requirements.memoryTypeBits, memoryFlags);

    auto it = m_memoryTable.find(hostMemoryType);

    if (it == m_memoryTable.end()) {
        m_memoryTable.insert(std::pair<uint32_t, std::vector<DeviceMemory>>(hostMemoryType, std::vector<DeviceMemory>()));
        it = m_memoryTable.find(hostMemoryType);
    }

    std::vector<DeviceMemory>& deviceMemories = it->second;

    std::pair<vk::DeviceMemory*, uint32_t> memoryData(nullptr, -1);

    uint32_t chunkIndex = 0;
    for (auto deviceMemory = deviceMemories.begin(); deviceMemory != deviceMemories.end(); ++deviceMemory) {
        memoryData = deviceMemory->allocateBlock(requirements.size, requirements.alignment);
        if (memoryData.second != -1) {
            return AllocId{memoryData.first, requirements.memoryTypeBits, chunkIndex, memoryData.second};
        }
        ++chunkIndex;
    }

    deviceMemories.push_back(DeviceMemory(m_logicalDevice, hostMemoryType));

    memoryData = deviceMemories.back().allocateBlock(requirements.size, requirements.alignment);

    return AllocId{memoryData.first, requirements.memoryTypeBits, chunkIndex, memoryData.second};
}

void MemoryAllocator::free(AllocId& allocId) {
    if (instance) {
        m_memoryTable[allocId.type][allocId.chunk].freeBlock(allocId.offset);
    }
}

void MemoryAllocator::freeAllMemory() {
    for (auto& memoryType : m_memoryTable) {
        for (DeviceMemory& memoryChunk : memoryType.second) {
            m_logicalDevice.freeMemory(memoryChunk.get());
        }
    }
}

void MemoryAllocator::destroy() {
    if (instance) {
        instance->freeAllMemory();
        delete instance;
        instance = nullptr;
    }
}

MemoryAllocator*  MemoryAllocator::instance = nullptr;

MemoryAllocator::MemoryAllocator(vk::Device& logicalDevice,
    const vk::PhysicalDeviceMemoryProperties& memoryProperties) :
    m_logicalDevice(logicalDevice),
    m_memoryProperties(memoryProperties) {}

uint32_t MemoryAllocator::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags propertyFlags) {
    for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; ++i) {
        if ((typeFilter & (1 << i)) && (m_memoryProperties.memoryTypes[i].propertyFlags & propertyFlags)) {
            return i;
        }
    }

    throw std::runtime_error("No suitable memory type found!");
}