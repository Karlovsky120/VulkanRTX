#pragma once

#include "GlobalDefines.h"

#include "Buffer.h"
#include "Vertex.h"

#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>
#include "VulkanInclude.h"

class Mesh {
public:
	Mesh(
		vk::Device& logicalDevice,
		std::vector<uint32_t> indices,
		std::string name,
		glm::vec3 position = glm::vec3(0.0f),
		glm::vec3 rotation = glm::vec3(0.0f),
		glm::vec3 scale = glm::vec3(1.0f));

	vk::Buffer& getIndexBuffer();
	Buffer& getIndexBufferObject();
	uint32_t getIndexCount();

	glm::vec3 getPosition();

	void translate(glm::vec3 offset);
	void rotate(glm::vec3 rotation);
	void scale(glm::vec3 scale);

	glm::mat4 getMeshMatrix();

private:
	Buffer m_deviceIndexBuffer;
	std::vector<uint32_t> m_indices;

	glm::vec3 m_position;
	glm::quat m_rotation;
	glm::vec3 m_scale;
};

