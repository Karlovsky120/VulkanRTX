#include "MemoryAllocator.h"

#include "DeviceMemory.h"

void MemoryAllocator::init(
    std::shared_ptr<MemoryAllocator>& storage,
    const vk::Device& logicalDevice,
    const vk::PhysicalDeviceMemoryProperties& memoryProperties) {

    storage = std::make_shared<MemoryAllocator>(logicalDevice, memoryProperties);
    instance = storage;
}

std::unique_ptr<AllocId> MemoryAllocator::allocate(
    vk::MemoryRequirements& requirements,
    vk::MemoryPropertyFlags memoryFlags,
    vk::MemoryAllocateFlags allocateFlags) {

    uint32_t deviceMemoryType = instance.lock()->findMemoryType(requirements.memoryTypeBits, memoryFlags);
    auto memoryId = std::make_pair(deviceMemoryType, allocateFlags);

    auto& memoryTable = instance.lock()->m_memoryTable;
    auto it = memoryTable.find(memoryId);

    if (it == memoryTable.end()) {
        memoryTable.insert(std::make_pair(memoryId, std::vector<DeviceMemory>()));
        it = memoryTable.find(memoryId);
    }

    std::vector<DeviceMemory>& deviceMemories = it->second;

    std::pair<vk::DeviceMemory*, uint32_t> memoryData(nullptr, -1);

    uint32_t chunkIndex = 0;
    for (auto deviceMemory = deviceMemories.begin(); deviceMemory != deviceMemories.end(); ++deviceMemory) {
        memoryData = deviceMemory->allocateBlock(requirements.size, requirements.alignment);
        if (memoryData.second != -1) {
            return std::make_unique<AllocId>(memoryData.first, deviceMemoryType, allocateFlags, chunkIndex, memoryData.second);
        }
        ++chunkIndex;
    }

    deviceMemories.push_back(DeviceMemory(instance.lock()->m_logicalDevice, deviceMemoryType, allocateFlags));

    memoryData = deviceMemories.back().allocateBlock(requirements.size, requirements.alignment);

    return std::make_unique<AllocId>(memoryData.first, deviceMemoryType, allocateFlags, chunkIndex, memoryData.second);
}

void MemoryAllocator::free(AllocId& allocId) {
    instance.lock()->m_memoryTable[std::make_pair(allocId.type, allocId.allocateFlags)][allocId.chunk].freeBlock(allocId.offset);
}

void MemoryAllocator::freeAllMemory() {
    for (auto& memoryType : m_memoryTable) {
        for (DeviceMemory& memoryChunk : memoryType.second) {
            m_logicalDevice.freeMemory(memoryChunk.get());
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
    const vk::Device& logicalDevice,
    const vk::PhysicalDeviceMemoryProperties& memoryProperties) :
    m_memoryProperties(memoryProperties),
    m_logicalDevice(logicalDevice) {}

std::weak_ptr<MemoryAllocator> MemoryAllocator::instance;

MemoryAllocator::~MemoryAllocator() {
    freeAllMemory();
}