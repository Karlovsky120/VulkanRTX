#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/vec3.hpp>

class VertexInputDescription {
public:
	vk::VertexInputBindingDescription& getBindingDescription();
	std::vector<vk::VertexInputAttributeDescription>& getAttributeDescriptions();

	uint32_t getAttributeCount();

	VertexInputDescription();

private:
	vk::VertexInputBindingDescription m_bindingDescription;
	std::vector<vk::VertexInputAttributeDescription> m_attributeDescriptions;

	struct Vertex {
		glm::vec3 m_pos;
		glm::vec3 m_color;
	};
};

