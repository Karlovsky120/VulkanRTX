#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/vec3.hpp>

struct Vertex {
	glm::vec3 m_pos;
	glm::vec3 m_color;
};

class Pipeline {
public:
	void createPipeline(const vk::Extent2D& extent);
	void createRenderPass(const vk::Format& format);
	void createPipelineLayout(const vk::DescriptorSetLayout* setLayout);

	void initPipeline(
		const vk::Extent2D& extent,
		const vk::Format& format,
		const vk::DescriptorSetLayout* setLayout);

	void updatePipeline(const vk::Format& format, const vk::Extent2D& extent);

	Pipeline(vk::Device& logicalDevice);

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;
	vk::UniqueRenderPass m_renderPass;

private:
	vk::Device& m_logicalDevice;
	vk::UniquePipelineCache m_pipelineCache;

	struct PipelineInfo {
		vk::VertexInputBindingDescription bindingDescription;
		std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
		vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo;
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo;

		vk::Viewport viewport;
		vk::Rect2D scissor;
		vk::PipelineViewportStateCreateInfo viewportCreateInfo;
		vk::PipelineRasterizationStateCreateInfo rasterizerationCreateInfo;
		vk::PipelineMultisampleStateCreateInfo multisampleCreateInfo;
		vk::PipelineColorBlendAttachmentState colorBlendAttachment;
		vk::PipelineColorBlendStateCreateInfo colorBlendCreateInfo;

		vk::UniqueShaderModule vertexShaderModule;
		vk::PipelineShaderStageCreateInfo vertShaderStageCreateInfo;
		vk::UniqueShaderModule fragmentShaderModule;
		vk::PipelineShaderStageCreateInfo fragShaderStageCreateInfo;
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

		vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
	};

	PipelineInfo m_pipelineInfo;

	vk::PipelineCacheCreateInfo m_cacheCreateInfo;

	struct RenderPassInfo {
		vk::AttachmentDescription colorAttachment;
		vk::AttachmentReference colorAttachmentRef;
		vk::SubpassDescription subpass;
		vk::SubpassDependency dependency;
		vk::RenderPassCreateInfo renderPassCreateInfo;
	};

	RenderPassInfo m_renderPassInfo;

	vk::UniqueShaderModule createShaderModule(const std::string shaderPath) const;
};

