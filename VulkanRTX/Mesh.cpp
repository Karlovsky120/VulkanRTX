#include "Mesh.h"

#include <fstream>

Mesh::Mesh(std::vector<uint32_t> indices,
		   std::string name,
		   glm::vec3 position,
		   glm::vec3 rotation,
		   glm::vec3 scale) :
	m_indices(indices),
	m_position(position),
	m_rotation(rotation),
	m_scale(scale),
	m_deviceIndexBuffer(sizeof(indices[0])* indices.size(),
		vk::BufferUsageFlagBits::eTransferDst
		| vk::BufferUsageFlagBits::eIndexBuffer
		| vk::BufferUsageFlagBits::eRayTracingKHR
		| vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		name,
		vk::MemoryAllocateFlagBits::eDeviceAddress) {

	m_deviceIndexBuffer.uploadToBuffer(indices);
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

glm::vec3 Mesh::getPosition() {
	return m_position;
}

glm::mat4 Mesh::getMeshMatrix() {
	glm::mat4 identity(1.0f);

	glm::mat4 scale = glm::scale(identity, m_scale);
	glm::mat4 rotate = glm::mat4_cast(m_rotation);
	glm::mat4 translate = glm::translate(identity, m_position);

	return translate * rotate * scale;
}