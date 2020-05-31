#include "AccelerationStructure.h"

#include "VulkanContext.h"

AccelerationStructure AccelerationStructures::createBottomAccelerationStructure(Mesh& mesh, vk::Buffer& vertices) {
	vk::AccelerationStructureCreateGeometryTypeInfoKHR geometryTypeInfo;
	geometryTypeInfo.geometryType = vk::GeometryTypeKHR::eTriangles;
	geometryTypeInfo.maxPrimitiveCount = mesh.getIndexCount() / 3;
	geometryTypeInfo.indexType = vk::IndexType::eUint32;
	geometryTypeInfo.maxVertexCount = 33 * 33 * 33;
	geometryTypeInfo.vertexFormat = vk::Format::eR32G32B32Sfloat;
	geometryTypeInfo.allowsTransforms = VK_TRUE;

	vk::AccelerationStructureCreateInfoKHR createInfo;
	createInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
	createInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	createInfo.maxGeometryCount = 1;
	createInfo.pGeometryInfos = &geometryTypeInfo;

	AccelerationStructure as;
	as.structure = VulkanContext::getDevice().createAccelerationStructureKHRUnique(createInfo);
	as.position = mesh.getPosition();

	NAME_OBJECT(
		&*as.structure,
		vk::DebugReportObjectTypeEXT::eAccelerationStructureKHR,
		"Bottom acceleration structure");

	vk::AccelerationStructureMemoryRequirementsInfoKHR memoryInfo;
	memoryInfo.type = vk::AccelerationStructureMemoryRequirementsTypeKHR::eObject;
	memoryInfo.buildType = vk::AccelerationStructureBuildTypeKHR::eDevice;
	memoryInfo.accelerationStructure = *as.structure;

	vk::MemoryRequirements memoryRequirements =
		VulkanContext::getDevice().getAccelerationStructureMemoryRequirementsKHR(memoryInfo).memoryRequirements;

	as.memory = MemoryAllocator::allocate(
		memoryRequirements,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::MemoryAllocateFlagBits::eDeviceAddress);

	vk::BindAccelerationStructureMemoryInfoKHR bindInfo;
	bindInfo.accelerationStructure = *as.structure;
	bindInfo.memory = as.memory->memory();
	bindInfo.memoryOffset = as.memory->offset;

	VulkanContext::getDevice().bindAccelerationStructureMemoryKHR(bindInfo);

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

	Buffer scratchBuffer = createScratchBuffer(
		*as.structure,
		vk::AccelerationStructureMemoryRequirementsTypeKHR::eBuildScratch,
		"Bottom AS scratch buffer");

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
	computeCmdBuffer->submitAndWait();

	vk::AccelerationStructureDeviceAddressInfoKHR addressInfo;
	addressInfo.accelerationStructure = *as.structure;
	as.address.deviceAddress =
		VulkanContext::getDevice().getAccelerationStructureAddressKHR(addressInfo);

	return as;
}

AccelerationStructure AccelerationStructures::createTopAccelerationStructure(
	std::vector<std::unique_ptr<AccelerationStructure>>& blases) {

	vk::AccelerationStructureCreateGeometryTypeInfoKHR geometryTypeInfo;
	geometryTypeInfo.geometryType = vk::GeometryTypeKHR::eInstances;
	geometryTypeInfo.maxPrimitiveCount = static_cast<uint32_t>(blases.size());
	geometryTypeInfo.allowsTransforms = VK_FALSE;

	vk::AccelerationStructureCreateInfoKHR createInfo;
	createInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
	createInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	createInfo.maxGeometryCount = 1;
	createInfo.pGeometryInfos = &geometryTypeInfo;

	AccelerationStructure as;
	as.structure = VulkanContext::getDevice().createAccelerationStructureKHRUnique(createInfo);

	NAME_OBJECT(
		&*as.structure,
		vk::DebugReportObjectTypeEXT::eAccelerationStructureKHR,
		"Top acceleration structure")

	vk::AccelerationStructureMemoryRequirementsInfoKHR memoryInfo;
	memoryInfo.type = vk::AccelerationStructureMemoryRequirementsTypeKHR::eObject;
	memoryInfo.buildType = vk::AccelerationStructureBuildTypeKHR::eDevice;
	memoryInfo.accelerationStructure = *as.structure;

	vk::MemoryRequirements memoryRequirements =
		VulkanContext::getDevice().getAccelerationStructureMemoryRequirementsKHR(memoryInfo).memoryRequirements;

	as.memory = MemoryAllocator::allocate(
		memoryRequirements,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::MemoryAllocateFlagBits::eDeviceAddress);

	vk::BindAccelerationStructureMemoryInfoKHR bindInfo;
	bindInfo.accelerationStructure = *as.structure;
	bindInfo.memory = as.memory->memory();
	bindInfo.memoryOffset = as.memory->offset;

	VulkanContext::getDevice().bindAccelerationStructureMemoryKHR(bindInfo);

	vk::TransformMatrixKHR transformation = std::array<std::array<float, 4>, 3> {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f
	};

	std::vector<vk::AccelerationStructureInstanceKHR> instances;

	for (uint32_t i = 0; i < blases.size(); ++i) {
		transformation.matrix[0][3] = blases[i]->position.x;
		transformation.matrix[1][3] = blases[i]->position.y;
		transformation.matrix[2][3] = blases[i]->position.z;

		vk::AccelerationStructureInstanceKHR instance;
		instance.transform = transformation;
		instance.instanceCustomIndex = i;
		instance.mask = 0xFF;
		instance.instanceShaderBindingTableRecordOffset = 0;
		instance.flags = VkGeometryInstanceFlagBitsKHR(vk::GeometryInstanceFlagBitsKHR::eTriangleCullDisable);
		instance.accelerationStructureReference = blases[i]->address.deviceAddress;

		instances.push_back(instance);
	}

	Buffer instanceBuffer = Buffer(
		instances.size() * sizeof(vk::AccelerationStructureInstanceKHR),
		vk::BufferUsageFlagBits::eRayTracingKHR
		| vk::BufferUsageFlagBits::eShaderDeviceAddress
		| vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		"Top AS instance buffer",
		vk::MemoryAllocateFlagBits::eDeviceAddress);

	instanceBuffer.uploadToBuffer(instances);

	vk::AccelerationStructureGeometryKHR geometry;
	geometry.geometryType = vk::GeometryTypeKHR::eInstances;
	geometry.geometry.instances.arrayOfPointers = VK_FALSE;
	geometry.geometry.instances.data.deviceAddress =
		getBufferDeviceAddress<vk::DeviceOrHostAddressConstKHR>(instanceBuffer.get()).deviceAddress;
	geometry.flags = vk::GeometryFlagBitsKHR::eOpaque;

	vk::AccelerationStructureGeometryKHR* pGeometry = &geometry;

	Buffer scratchBuffer = createScratchBuffer(
		*as.structure,
		vk::AccelerationStructureMemoryRequirementsTypeKHR::eBuildScratch,
		"Top AS scratch buffer");

	vk::AccelerationStructureBuildGeometryInfoKHR geometryInfo;
	geometryInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
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
	offsetInfo.primitiveCount = blases.size();
	offsetInfo.primitiveOffset = 0;
	offsetInfo.firstVertex = 0;
	offsetInfo.transformOffset = 0;

	vk::AccelerationStructureBuildOffsetInfoKHR* pOffsetInfo = &offsetInfo;

	std::unique_ptr<CommandBuffer> computeCmdBuffer = std::make_unique<CommandBuffer>(PoolType::eCompute);
	computeCmdBuffer->get().buildAccelerationStructureKHR(1, &geometryInfo, &pOffsetInfo);
	computeCmdBuffer->submitAndWait();

	vk::AccelerationStructureDeviceAddressInfoKHR addressInfo;
	addressInfo.accelerationStructure = *as.structure;
	as.address.deviceAddress =
		VulkanContext::getDevice().getAccelerationStructureAddressKHR(addressInfo);

	return as;
}

Buffer AccelerationStructures::createScratchBuffer(
	vk::AccelerationStructureKHR& accelerationStructure,
	vk::AccelerationStructureMemoryRequirementsTypeKHR type,
	std::string name) {

	vk::AccelerationStructureMemoryRequirementsInfoKHR memoryInfo;
	memoryInfo.type = type;
	memoryInfo.buildType = vk::AccelerationStructureBuildTypeKHR::eDevice;
	memoryInfo.accelerationStructure = accelerationStructure;

	vk::MemoryRequirements memoryRequirements =
		VulkanContext::getDevice().getAccelerationStructureMemoryRequirementsKHR(memoryInfo).memoryRequirements;

	return Buffer(
		memoryRequirements.size,
		vk::BufferUsageFlagBits::eRayTracingKHR
		| vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		name,
		vk::MemoryAllocateFlagBits::eDeviceAddress);
}