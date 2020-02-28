#pragma once

#include "Buffer.h"

#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vulkan/vulkan.hpp>

class Mesh {
public:
	Mesh(vk::Device& logicalDevice,
		 std::vector<float> vertices,
		 std::vector<uint16_t> indices,
		 glm::vec3 position = glm::vec3(0.0f),
		 glm::vec3 rotation = glm::vec3(0.0f),
		 glm::vec3 scale = glm::vec3(1.0f));

	void recordUploadToGPU(vk::CommandBuffer& transferBuffer);

	vk::Buffer& getVertexBuffer();
	vk::Buffer& getIndexBuffer();

	void translate(glm::vec3 offset);
	void rotate(glm::quat rotation);
	void scale(glm::vec3 scale);

	glm::mat4 getMeshMatrix();

private:
	Buffer m_hostVertexBuffer;
	Buffer m_hostIndexBuffer;
	Buffer m_deviceVertexBuffer;
	Buffer m_deviceIndexBuffer;
	std::vector<float> m_vertices;
	std::vector<uint16_t> m_indices;

	glm::vec3 m_position;
	glm::quat m_rotation;
	glm::vec3 m_scale;
};
