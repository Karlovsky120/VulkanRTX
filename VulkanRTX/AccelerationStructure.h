#pragma once

#include "GlobalDefines.h"

#include "Buffer.h"
#include "Mesh.h"

#include "VulkanInclude.h"

struct AccelerationStructure {
	vk::UniqueAccelerationStructureKHR structure;
	std::unique_ptr<AllocId>  memory;
	vk::DeviceOrHostAddressKHR address;
	glm::vec3 position;
};

class AccelerationStructures {
public:
	AccelerationStructure createBottomAccelerationStructure(Mesh& mesh, vk::Buffer& vertices);
	AccelerationStructure createTopAccelerationStructure(std::vector<std::unique_ptr<AccelerationStructure>>& blase);

private:
	template <class T>
	T getBufferDeviceAddress(vk::Buffer& buffer) {
		vk::BufferDeviceAddressInfo addressInfo;
		addressInfo.buffer = buffer;

		return { VulkanContext::getDevice().getBufferAddress(addressInfo) };
	}

	Buffer createScratchBuffer(
		vk::AccelerationStructureKHR& accelerationStructure,
		vk::AccelerationStructureMemoryRequirementsTypeKHR type,
		std::string name
	);
};

