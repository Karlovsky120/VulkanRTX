#include "MemoryAllocator.h"

#include "DeviceMemory.h"

bool MemoryAllocator::initialized = false;
vk::Device* MemoryAllocator::m_logicalDevice;
std::map<uint32_t, std::vector<DeviceMemory>> MemoryAllocator::m_memoryTable;
vk::PhysicalDeviceMemoryProperties MemoryAllocator::m_memoryProperties;

void MemoryAllocator::init(vk::PhysicalDevice* physicalDevice, vk::Device* logicalDevice) {
    m_logicalDevice = logicalDevice;
    m_memoryProperties = physicalDevice->getMemoryProperties();
    initialized = true;
}

void MemoryAllocator::deinit() {
    for (auto& memoryType : m_memoryTable) {
        for (auto& memoryChunk : memoryType.second) {
            m_logicalDevice->freeMemory(memoryChunk.get());
        }
    }
}

AllocId MemoryAllocator::allocate(vk::MemoryRequirements& requirements, vk::MemoryPropertyFlags memoryFlags) {
    assert(initialized);

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

    deviceMemories.push_back(DeviceMemory(*m_logicalDevice, hostMemoryType));

    memoryData = deviceMemories.back().allocateBlock(requirements.size, requirements.alignment);

    return AllocId{memoryData.first, requirements.memoryTypeBits, chunkIndex, memoryData.second};
}

void MemoryAllocator::free(AllocId& allocId) {
    assert(initialized);
    m_memoryTable[allocId.type][allocId.chunk].freeBlock(allocId.offset);
}

uint32_t MemoryAllocator::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags propertyFlags) {
    for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; ++i) {
        if ((typeFilter & (1 << i)) && (m_memoryProperties.memoryTypes[i].propertyFlags & propertyFlags)) {
            return i;
        }
    }
}