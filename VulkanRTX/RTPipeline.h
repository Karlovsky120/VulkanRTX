#pragma once

#include "GlobalDefines.h"

#include "VulkanInclude.h"

#define INDEX_RAYGEN 0
#define INDEX_MISS 1
#define INDEX_SHADOW_MISS 2
#define INDEX_CLOSEST_HIT 3

class RTPipeline {
public:
	void createPipeline(vk::DescriptorSetLayout& setLayout);

	vk::Pipeline& get();
	vk::PipelineLayout& getLayout();
	
private:
	vk::UniqueShaderModule createShaderModule(
		const std::string shaderPath) const;

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineCache m_pipelineCache;
	vk::UniquePipelineLayout m_pipelineLayout;

	vk::UniqueDescriptorSetLayout m_descriptorSetLayout;
};

