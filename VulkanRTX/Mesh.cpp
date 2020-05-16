#include "Mesh.h"

#include <fstream>

Mesh::Mesh(vk::Device& logicalDevice,
		   std::vector<Vertex> vertices,
		   uint32_t vertexStride,
		   std::vector<uint16_t> indices,
		   glm::vec3 position,
		   glm::vec3 rotation,
		   glm::vec3 scale) :
	m_vertices(vertices),
	m_vertexStride(vertexStride),
	m_indices(indices),
	m_position(position),
	m_rotation(rotation),
	m_scale(scale),
	m_deviceVertexBuffer(logicalDevice,
		sizeof(vertices[0]) * vertices.size(),
		vk::BufferUsageFlagBits::eTransferDst
		| vk::BufferUsageFlagBits::eVertexBuffer
		| vk::BufferUsageFlagBits::eRayTracingKHR
		| vk::BufferUsageFlagBits::eShaderDeviceAddress, //so it can be used with getBufferAddress()
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::MemoryAllocateFlagBits::eDeviceAddress), //so it can be used with getBufferAddress()
	m_deviceIndexBuffer(logicalDevice,
		sizeof(indices[0])* indices.size(),
		vk::BufferUsageFlagBits::eTransferDst
		| vk::BufferUsageFlagBits::eIndexBuffer
		| vk::BufferUsageFlagBits::eRayTracingKHR
		| vk::BufferUsageFlagBits::eShaderDeviceAddress, //so it can be used with getBufferAddress()
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::MemoryAllocateFlagBits::eDeviceAddress) { //so it can be used with getBufferAddress()

	m_deviceVertexBuffer.uploadToBuffer(vertices);
	m_deviceIndexBuffer.uploadToBuffer(indices);
}

vk::Buffer& Mesh::getVertexBuffer() {
	return m_deviceVertexBuffer.get();
}

Buffer& Mesh::getVertexBufferObject() {
	return m_deviceVertexBuffer;
}

uint32_t Mesh::getVertexCount() {
	return static_cast<uint32_t>(m_vertices.size());
}

vk::Buffer& Mesh::getIndexBuffer() {
	return m_deviceIndexBuffer.get();
}

Buffer& Mesh::getIndexBufferObject() {
	return m_deviceIndexBuffer;
}

uint32_t Mesh::getIndexCount() {
	return static_cast<uint32_t>(m_indices.size());
}

void Mesh::translate(glm::vec3 offset) {
	m_position += offset;
}

void Mesh::rotate(glm::vec3 rotation) {
	m_rotation = m_rotation * glm::quat(rotation);
}

void Mesh::scale(glm::vec3 scale) {
	m_scale *= scale;
}

glm::mat4 Mesh::getMeshMatrix() {
	glm::mat4 identity(1.0f);

	glm::mat4 scale = glm::scale(identity, m_scale);
	glm::mat4 rotate = glm::mat4_cast(m_rotation);
	glm::mat4 translate = glm::translate(identity, m_position);

	return translate * rotate * scale;
}