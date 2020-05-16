#pragma once

#include "Buffer.h"
#include "Mesh.h"

#include "VulkanInclude.h"

struct AccelerationStructure {
	vk::UniqueAccelerationStructureKHR structure;
	std::unique_ptr<AllocId>  memory;
	vk::DeviceOrHostAddressKHR address;
};

class AccelerationStructures {
public:
	AccelerationStructure createBottomAccelerationStructure(Mesh& mesh);
	AccelerationStructure createTopAccelerationStructure(AccelerationStructure& blas);

private:
	template <class T>
	T getBufferDeviceAddress(vk::Buffer& buffer) {
		vk::BufferDeviceAddressInfo addressInfo;
		addressInfo.buffer = buffer;

		return { VulkanContext::get()->m_logicalDevice->getBufferAddress(addressInfo) };
	}

	Buffer createScratchBuffer(
		vk::AccelerationStructureKHR& accelerationStructure,
		vk::AccelerationStructureMemoryRequirementsTypeKHR type
	);
};

