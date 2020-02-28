#include "Mesh.h"


Mesh::Mesh(vk::Device& logicalDevice,
		   std::vector<float> vertices,
		   std::vector<uint16_t> indices,
		   glm::vec3 position,
		   glm::vec3 rotation,
		   glm::vec3 scale) :
	m_vertices(vertices),
	m_indices(indices),
	m_position(position),
	m_rotation(rotation),
	m_scale(scale),
	m_hostVertexBuffer(logicalDevice,
					   sizeof(float) * vertices.size(),
					   vk::BufferUsageFlagBits::eTransferSrc,
					   vk::MemoryPropertyFlagBits::eHostVisible),
	m_hostIndexBuffer(logicalDevice,
					  sizeof(uint16_t) * indices.size(),
					  vk::BufferUsageFlagBits::eTransferSrc,
					  vk::MemoryPropertyFlagBits::eHostVisible),
	m_deviceVertexBuffer(logicalDevice,
						 sizeof(float) * vertices.size(),
						 vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eVertexBuffer,
						 vk::MemoryPropertyFlagBits::eDeviceLocal),
	m_deviceIndexBuffer(logicalDevice,
						 sizeof(uint16_t)* indices.size(),
						 vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eIndexBuffer,
						 vk::MemoryPropertyFlagBits::eDeviceLocal) {

	m_hostVertexBuffer.copyToBuffer(vertices);
	m_hostIndexBuffer.copyToBuffer(indices);
}

void Mesh::recordUploadToGPU(vk::CommandBuffer& transferBuffer) {
	vk::BufferCopy bufferCopy;

	bufferCopy.size = sizeof(float) * m_vertices.size();
	bufferCopy.srcOffset = 0;
	bufferCopy.dstOffset = 0;

	transferBuffer.copyBuffer(m_hostVertexBuffer.get(), m_deviceVertexBuffer.get(), bufferCopy);

	bufferCopy.size = sizeof(uint16_t) * m_indices.size();
	bufferCopy.srcOffset = 0;
	bufferCopy.dstOffset = 0;

	transferBuffer.copyBuffer(m_hostIndexBuffer.get(), m_deviceIndexBuffer.get(), bufferCopy);
}

vk::Buffer& Mesh::getVertexBuffer() {
	return m_deviceVertexBuffer.get();
}

vk::Buffer& Mesh::getIndexBuffer() {
	return m_deviceIndexBuffer.get();
}

void Mesh::translate(glm::vec3 offset) {
	m_position += offset;
}

void Mesh::rotate(glm::quat rotation) {
	m_rotation = m_rotation * rotation;
}

void Mesh::scale(glm::vec3 scale) {
	m_scale *= scale;
}

glm::mat4 Mesh::getMeshMatrix() {
	glm::mat4 meshMatrix(1.0f);

	meshMatrix = glm::scale(meshMatrix, m_scale);
	meshMatrix = glm::mat4_cast(m_rotation) * meshMatrix;
	meshMatrix = glm::translate(meshMatrix, m_position);

	return meshMatrix;
}