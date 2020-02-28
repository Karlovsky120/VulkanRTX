#include "VertexInputDescription.h"

vk::VertexInputBindingDescription& VertexInputDescription::getBindingDescription() {
	return m_bindingDescription;
}

std::vector<vk::VertexInputAttributeDescription>& VertexInputDescription::getAttributeDescriptions() {
	return m_attributeDescriptions;
}

uint32_t VertexInputDescription::getAttributeCount() {
	return m_attributeDescriptions.size();
}

VertexInputDescription::VertexInputDescription() {
	m_bindingDescription.binding = 0;
	m_bindingDescription.stride = sizeof(Vertex);
	m_bindingDescription.inputRate = vk::VertexInputRate::eVertex;

	m_attributeDescriptions = std::vector<vk::VertexInputAttributeDescription>(2);

	m_attributeDescriptions[0].binding = 0;
	m_attributeDescriptions[0].location = 0;
	m_attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
	m_attributeDescriptions[0].offset = offsetof(Vertex, m_pos);

	m_attributeDescriptions[1].binding = 0;
	m_attributeDescriptions[1].location = 1;
	m_attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
	m_attributeDescriptions[1].offset = offsetof(Vertex, m_color);
}
