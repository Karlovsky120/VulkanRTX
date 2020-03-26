#include "RayTracing.h"

#include "Buffer.h"
#include "CmdBufferAllocator.h"

#include <vector>

vk::UniqueAccelerationStructureKHR RayTracing::createBottomAccelerationStructure(Mesh& mesh) {
	vk::AccelerationStructureCreateGeometryTypeInfoKHR geometryTypeInfo;
	geometryTypeInfo.geometryType = vk::GeometryTypeKHR::eTriangles;
	geometryTypeInfo.maxPrimitiveCount = mesh.getVertextCount() / 3;
	geometryTypeInfo.indexType = vk::IndexType::eUint16;
	geometryTypeInfo.maxVertexCount = mesh.getVertextCount();
	geometryTypeInfo.vertexFormat = vk::Format::eR32G32B32Sfloat;
	geometryTypeInfo.allowsTransforms = VK_TRUE;

	vk::AccelerationStructureCreateInfoKHR createInfo;
	createInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
	createInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	createInfo.maxGeometryCount = 1;
	createInfo.pGeometryInfos = &geometryTypeInfo;

	vk::UniqueAccelerationStructureKHR accelerationStructure =
		m_context->m_logicalDevice->createAccelerationStructureKHRUnique(createInfo);

	float rowMajorAffineTransformation[3][4] =
	{{1.0f, 0.0f, 0.0f, 0.0f},
	{0.0f, 1.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 1.0f, 0.0f}};

	vk::TransformMatrixKHR transformation;
	memcpy(transformation.matrix, rowMajorAffineTransformation, 3 * 4 * sizeof(float));

	vk::DeviceOrHostAddressConstKHR transformAddress;
	transformAddress.hostAddress = &transformation;

	vk::AccelerationStructureGeometryTrianglesDataKHR triangleData;
	triangleData.vertexFormat = vk::Format::eR32G32B32Sfloat;
	triangleData.vertexData = getBufferDeviceAddress<vk::DeviceOrHostAddressConstKHR>(mesh.getVertexBuffer());
	triangleData.vertexStride = vk::DeviceSize(sizeof(float) * 3);
	triangleData.indexType = vk::IndexType::eUint16;
	triangleData.indexData = getBufferDeviceAddress<vk::DeviceOrHostAddressConstKHR>(mesh.getIndexBuffer());
	triangleData.transformData = transformAddress;

	vk::AccelerationStructureGeometryDataKHR geometryData;
	geometryData.triangles = triangleData;

	vk::AccelerationStructureGeometryKHR geometry;
	geometry.geometryType = vk::GeometryTypeKHR::eTriangles;
	geometry.geometry = geometryData;
	geometry.flags = vk::GeometryFlagBitsKHR::eOpaque;

	const vk::AccelerationStructureGeometryKHR* const pGeometry = &geometry;

	Buffer scratchBuffer = createScratchBuffer(*accelerationStructure);

	vk::AccelerationStructureBuildGeometryInfoKHR geometryInfo;
	geometryInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
	geometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	geometryInfo.update = VK_FALSE;
	geometryInfo.srcAccelerationStructure = nullptr;
	geometryInfo.dstAccelerationStructure = *accelerationStructure;
	geometryInfo.geometryArrayOfPointers = VK_FALSE;
	geometryInfo.geometryCount = 1;
	geometryInfo.ppGeometries = &pGeometry;
	geometryInfo.scratchData = getBufferDeviceAddress<vk::DeviceOrHostAddressKHR>(scratchBuffer.get());

	vk::AccelerationStructureBuildOffsetInfoKHR offsetInfo;
	offsetInfo.primitiveCount = mesh.getVertextCount() / 3;
	offsetInfo.primitiveOffset = 0;
	offsetInfo.firstVertex = 0;
	offsetInfo.transformOffset = 0;

	vk::AccelerationStructureBuildOffsetInfoKHR* pOffsetInfo = &offsetInfo;

	vk::UniqueCommandBuffer computeCmdBuffer = CmdBufferAllocator::get()->createBufferUnique(*CmdBufferAllocator::get()->m_computeCmdPool);
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	computeCmdBuffer->begin(beginInfo);
	computeCmdBuffer->buildAccelerationStructureKHR(1, &geometryInfo, &pOffsetInfo);
	computeCmdBuffer->end();

	CmdBufferAllocator::get()->submitBuffer(*computeCmdBuffer, m_context->m_computeQueue, true);

	return std::move(accelerationStructure);
}

vk::UniqueAccelerationStructureKHR RayTracing::createTopAccelerationStructure(vk::AccelerationStructureKHR& bottom) {
	vk::AccelerationStructureCreateGeometryTypeInfoKHR geometryTypeInfo;
	geometryTypeInfo.geometryType = vk::GeometryTypeKHR::eInstances;
	
	vk::AccelerationStructureCreateInfoKHR createInfo;
	createInfo.type = vk::AccelerationStructureTypeKHR::eTopLevel;
	createInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	createInfo.maxGeometryCount = 1;
	createInfo.pGeometryInfos = &geometryTypeInfo;

	vk::UniqueAccelerationStructureKHR accelerationStructure = m_context->m_logicalDevice->createAccelerationStructureKHRUnique(createInfo);

	float rowMajorAffineTransformation[3][4] =
	{ {1.0f, 0.0f, 0.0f, 0.0f},
	{0.0f, 1.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 1.0f, 0.0f} };

	vk::TransformMatrixKHR transformation;
	memcpy(transformation.matrix, rowMajorAffineTransformation, 3 * 4 * sizeof(float));

	vk::AccelerationStructureDeviceAddressInfoKHR addressInfo;
	addressInfo.accelerationStructure = bottom;

	vk::DeviceAddress bottomAccelerationStructureAddress = m_context->m_logicalDevice->getAccelerationStructureAddressKHR(addressInfo);

	vk::AccelerationStructureInstanceKHR instance;
	instance.transform = transformation;
	instance.instanceCustomIndex = 0;
	instance.mask = 0;
	instance.instanceShaderBindingTableRecordOffset = 0;
	instance.flags = VkGeometryInstanceFlagBitsKHR(vk::GeometryInstanceFlagBitsKHR::eTriangleCullDisable);
	instance.accelerationStructureReference = bottomAccelerationStructureAddress;

	vk::DeviceOrHostAddressConstKHR instanceAddress;
	instanceAddress.hostAddress = &instance;

	vk::AccelerationStructureGeometryInstancesDataKHR instanceData;
	instanceData.arrayOfPointers = VK_FALSE;
	instanceData.data = instanceAddress;

	vk::AccelerationStructureGeometryDataKHR geometryData;
	geometryData.instances = instanceData;

	vk::AccelerationStructureGeometryKHR geometry;
	geometry.geometryType = vk::GeometryTypeKHR::eInstances;
	geometry.geometry = geometryData;
	geometry.flags = vk::GeometryFlagBitsKHR::eOpaque;

	vk::AccelerationStructureGeometryKHR* pGeometry = &geometry;

	Buffer scratchBuffer = createScratchBuffer(*accelerationStructure);

	vk::AccelerationStructureBuildGeometryInfoKHR geometryInfo;
	geometryInfo.type = vk::AccelerationStructureTypeKHR::eBottomLevel;
	geometryInfo.flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace;
	geometryInfo.update = VK_FALSE;
	geometryInfo.srcAccelerationStructure = nullptr;
	geometryInfo.dstAccelerationStructure = *accelerationStructure;
	geometryInfo.geometryArrayOfPointers = VK_FALSE;
	geometryInfo.geometryCount = 1;
	geometryInfo.ppGeometries = &pGeometry;
	geometryInfo.scratchData = getBufferDeviceAddress<vk::DeviceOrHostAddressKHR>(scratchBuffer.get());



	return std::move(accelerationStructure);
}

Buffer RayTracing::createScratchBuffer(vk::AccelerationStructureKHR& accelerationStructure) {
	vk::AccelerationStructureMemoryRequirementsInfoKHR memoryInfo;
	memoryInfo.type = vk::AccelerationStructureMemoryRequirementsTypeKHR::eBuildScratch;
	memoryInfo.buildType = vk::AccelerationStructureBuildTypeKHR::eDevice;
	memoryInfo.accelerationStructure = accelerationStructure;

	vk::MemoryRequirements memoryRequirements =
		m_context->m_logicalDevice->getAccelerationStructureMemoryRequirementsKHR(memoryInfo).memoryRequirements;

	return Buffer(
		*m_context->m_logicalDevice,
		memoryRequirements.size,
		vk::BufferUsageFlagBits::eRayTracingKHR
		| vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::MemoryAllocateFlagBits::eDeviceAddress,
		memoryRequirements);
}

RayTracing::RayTracing(std::shared_ptr<VulkanContext> vkCtx) :
	m_context(vkCtx) {};

