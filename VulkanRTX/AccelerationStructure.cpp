#include "AccelerationStructure.h"

#include "VulkanContext.h"

AccelerationStructure AccelerationStructures::createBottomAccelerationStructure(Mesh& mesh, vk::Buffer& vertices) {
	vk::AccelerationStructureCreateGeometryTypeInfoKHR geometryTypeInfo;
	geometryTypeInfo.geometryType = vk::GeometryTypeKHR::eTriangles;
	geometryTypeInfo.maxPrimitiveCount = mesh.getIndexCount() / 3;
	geometryTypeInfo.indexType = vk::IndexType::eUint32;
	geometryTypeInfo.maxVertexCount = 33 * 33 * 33; // mesh.getVertexCount();
	geometryTypeInfo.vertexFormat = vk::Format::eR32G32B32Sfloat;
	geometryTypeInfo.allowsTransforms = VK_FALSE;

	vk::AccelerationStructureCreateInfoKHR createInfo;
	createInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
	createInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	createInfo.maxGeometryCount = 1;
	createInfo.pGeometryInfos = &geometryTypeInfo;

	AccelerationStructure as;
	as.structure = VulkanContext::get()->m_logicalDevice->createAccelerationStructureKHRUnique(createInfo);

	vk::AccelerationStructureMemoryRequirementsInfoKHR memoryInfo;
	memoryInfo.type = vk::AccelerationStructureMemoryRequirementsTypeKHR::eObject;
	memoryInfo.buildType = vk::AccelerationStructureBuildTypeKHR::eDevice;
	memoryInfo.accelerationStructure = *as.structure;

	vk::MemoryRequirements memoryRequirements =
		VulkanContext::get()->m_logicalDevice->getAccelerationStructureMemoryRequirementsKHR(memoryInfo).memoryRequirements;

	as.memory = MemoryAllocator::allocate(memoryRequirements, vk::MemoryPropertyFlagBits::eDeviceLocal);

	vk::BindAccelerationStructureMemoryInfoKHR bindInfo;
	bindInfo.accelerationStructure = *as.structure;
	bindInfo.memory = *as.memory->memory;
	bindInfo.memoryOffset = as.memory->offset;

	VulkanContext::get()->m_logicalDevice->bindAccelerationStructureMemoryKHR(bindInfo);

	Buffer scratchBuffer = createScratchBuffer(
		*as.structure,
		vk::AccelerationStructureMemoryRequirementsTypeKHR::eBuildScratch);

	vk::AccelerationStructureGeometryKHR geometry;
	geometry.flags = vk::GeometryFlagBitsKHR::eOpaque;
	geometry.geometryType = vk::GeometryTypeKHR::eTriangles;
	geometry.geometry.triangles.vertexFormat = vk::Format::eR32G32B32Sfloat;
	geometry.geometry.triangles.vertexData =
		getBufferDeviceAddress<vk::DeviceOrHostAddressConstKHR>(vertices).deviceAddress;
	geometry.geometry.triangles.vertexStride = vk::DeviceSize(sizeof(Vertex));
	geometry.geometry.triangles.indexType = vk::IndexType::eUint32;
	geometry.geometry.triangles.indexData.deviceAddress =
		getBufferDeviceAddress<vk::DeviceOrHostAddressConstKHR>(mesh.getIndexBuffer()).deviceAddress;

	const vk::AccelerationStructureGeometryKHR* const pGeometry = &geometry;

	vk::AccelerationStructureBuildGeometryInfoKHR buildGeometryInfo;
	buildGeometryInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
	buildGeometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	buildGeometryInfo.update = VK_FALSE;
	buildGeometryInfo.srcAccelerationStructure = nullptr;
	buildGeometryInfo.dstAccelerationStructure = *as.structure;
	buildGeometryInfo.geometryArrayOfPointers = VK_FALSE;
	buildGeometryInfo.geometryCount = 1;
	buildGeometryInfo.ppGeometries = &pGeometry;
	buildGeometryInfo.scratchData.deviceAddress =
		getBufferDeviceAddress<vk::DeviceOrHostAddressKHR>(scratchBuffer.get()).deviceAddress;

	vk::AccelerationStructureBuildOffsetInfoKHR offsetInfo;
	offsetInfo.primitiveCount = mesh.getIndexCount() / 3;
	offsetInfo.primitiveOffset = 0;
	offsetInfo.firstVertex = 0;
	offsetInfo.transformOffset = 0;

	vk::AccelerationStructureBuildOffsetInfoKHR* pOffsetInfo = &offsetInfo;

	std::unique_ptr<CommandBuffer> computeCmdBuffer = std::make_unique<CommandBuffer>(PoolType::eCompute);
	computeCmdBuffer->get().buildAccelerationStructureKHR(1, &buildGeometryInfo, &pOffsetInfo);
	computeCmdBuffer->submit(true);

	vk::AccelerationStructureDeviceAddressInfoKHR addressInfo;
	addressInfo.accelerationStructure = *as.structure;
	as.address.deviceAddress =
		VulkanContext::get()->m_logicalDevice->getAccelerationStructureAddressKHR(addressInfo);

	return as;
}

AccelerationStructure AccelerationStructures::createTopAccelerationStructure(
	AccelerationStructure& blas) {

	vk::AccelerationStructureCreateGeometryTypeInfoKHR geometryTypeInfo;
	geometryTypeInfo.geometryType = vk::GeometryTypeKHR::eInstances;
	geometryTypeInfo.maxPrimitiveCount = 1;
	geometryTypeInfo.allowsTransforms = VK_FALSE;

	vk::AccelerationStructureCreateInfoKHR createInfo;
	createInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
	createInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	createInfo.maxGeometryCount = 1;
	createInfo.pGeometryInfos = &geometryTypeInfo;

	AccelerationStructure as;
	as.structure = VulkanContext::get()->m_logicalDevice->createAccelerationStructureKHRUnique(createInfo);

	vk::AccelerationStructureMemoryRequirementsInfoKHR memoryInfo;
	memoryInfo.type = vk::AccelerationStructureMemoryRequirementsTypeKHR::eObject;
	memoryInfo.buildType = vk::AccelerationStructureBuildTypeKHR::eDevice;
	memoryInfo.accelerationStructure = *as.structure;

	vk::MemoryRequirements memoryRequirements =
		VulkanContext::get()->m_logicalDevice->getAccelerationStructureMemoryRequirementsKHR(memoryInfo).memoryRequirements;

	as.memory = MemoryAllocator::allocate(
		memoryRequirements,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::MemoryAllocateFlagBits::eDeviceAddress);

	vk::BindAccelerationStructureMemoryInfoKHR bindInfo;
	bindInfo.accelerationStructure = *as.structure;
	bindInfo.memory = *as.memory->memory;
	bindInfo.memoryOffset = as.memory->offset;

	VulkanContext::get()->m_logicalDevice->bindAccelerationStructureMemoryKHR(bindInfo);

	vk::TransformMatrixKHR transformation;
	transformation.matrix = std::array<std::array<float, 4>, 3> {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f
	};

	vk::AccelerationStructureInstanceKHR instance;
	instance.transform = transformation;
	instance.instanceCustomIndex = 0;
	instance.mask = 0xFF;
	instance.instanceShaderBindingTableRecordOffset = 0;
	instance.flags = VkGeometryInstanceFlagBitsKHR(vk::GeometryInstanceFlagBitsKHR::eTriangleCullDisable);
	instance.accelerationStructureReference = blas.address.deviceAddress;

	Buffer instanceBuffer = Buffer(
		*VulkanContext::get()->m_logicalDevice,
		sizeof(instance),
		vk::BufferUsageFlagBits::eShaderDeviceAddress
		| vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::MemoryAllocateFlagBits::eDeviceAddress);

	std::vector<vk::AccelerationStructureInstanceKHR> instanceVec = { instance };
	instanceBuffer.uploadToBuffer(instanceVec);

	vk::DeviceOrHostAddressConstKHR instanceAddress;
	instanceAddress.deviceAddress =
		VulkanContext::get()->m_logicalDevice->getBufferAddress(instanceBuffer.get());

	vk::AccelerationStructureGeometryKHR geometry;
	geometry.geometryType = vk::GeometryTypeKHR::eInstances;
	geometry.geometry.instances.arrayOfPointers = VK_FALSE;
	geometry.geometry.instances.data = &instanceAddress.deviceAddress;
	geometry.flags = vk::GeometryFlagBitsKHR::eOpaque;

	vk::AccelerationStructureGeometryKHR* pGeometry = &geometry;

	Buffer scratchBuffer = createScratchBuffer(
		*as.structure,
		vk::AccelerationStructureMemoryRequirementsTypeKHR::eBuildScratch);

	vk::AccelerationStructureBuildGeometryInfoKHR geometryInfo;
	geometryInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
	geometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	geometryInfo.update = VK_FALSE;
	geometryInfo.srcAccelerationStructure = nullptr;
	geometryInfo.dstAccelerationStructure = *as.structure;
	geometryInfo.geometryArrayOfPointers = VK_FALSE;
	geometryInfo.geometryCount = 1;
	geometryInfo.ppGeometries = &pGeometry;
	geometryInfo.scratchData.deviceAddress =
		getBufferDeviceAddress<vk::DeviceOrHostAddressKHR>(scratchBuffer.get()).deviceAddress;

	vk::AccelerationStructureBuildOffsetInfoKHR offsetInfo;
	offsetInfo.primitiveCount = 1;
	offsetInfo.primitiveOffset = 0;
	offsetInfo.firstVertex = 0;
	offsetInfo.transformOffset = 0;

	vk::AccelerationStructureBuildOffsetInfoKHR* pOffsetInfo = &offsetInfo;

	std::unique_ptr<CommandBuffer> computeCmdBuffer = std::make_unique<CommandBuffer>(PoolType::eCompute);
	computeCmdBuffer->get().buildAccelerationStructureKHR(1, &geometryInfo, &pOffsetInfo);
	computeCmdBuffer->submit(true);

	vk::AccelerationStructureDeviceAddressInfoKHR addressInfo;
	addressInfo.accelerationStructure = *as.structure;
	as.address.deviceAddress =
		VulkanContext::get()->m_logicalDevice->getAccelerationStructureAddressKHR(addressInfo);

	return as;
}

Buffer AccelerationStructures::createScratchBuffer(
	vk::AccelerationStructureKHR& accelerationStructure,
	vk::AccelerationStructureMemoryRequirementsTypeKHR type) {

	vk::AccelerationStructureMemoryRequirementsInfoKHR memoryInfo;
	memoryInfo.type = type;
	memoryInfo.buildType = vk::AccelerationStructureBuildTypeKHR::eDevice;
	memoryInfo.accelerationStructure = accelerationStructure;

	vk::MemoryRequirements memoryRequirements =
		VulkanContext::get()->m_logicalDevice->getAccelerationStructureMemoryRequirementsKHR(memoryInfo).memoryRequirements;

	return Buffer(
		*VulkanContext::get()->m_logicalDevice,
		memoryRequirements.size,
		vk::BufferUsageFlagBits::eRayTracingKHR
		| vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::MemoryAllocateFlagBits::eDeviceAddress);
}