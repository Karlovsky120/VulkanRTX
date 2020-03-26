#pragma once

#include "VulkanInclude.h"

#include "Mesh.h"
#include "VulkanContext.h"

class RayTracing {
public:
	vk::UniqueAccelerationStructureKHR createBottomAccelerationStructure(Mesh& mesh);
	vk::UniqueAccelerationStructureKHR createTopAccelerationStructure(vk::AccelerationStructureKHR& bottom);

	RayTracing(std::shared_ptr<VulkanContext> vkCtx);

private:
	template <class T>
	T getBufferDeviceAddress(vk::Buffer& buffer) {
		vk::BufferDeviceAddressInfo addressInfo;
		addressInfo.buffer = buffer;

		return { m_context->m_logicalDevice->getBufferAddress(addressInfo) };
	}

	Buffer createScratchBuffer(vk::AccelerationStructureKHR& accelerationStructure);

	std::shared_ptr<VulkanContext> m_context;

};

