#pragma once

#include "CommandPools.h"
#include "VulkanInclude.h"

class VulkanContext;

class CommandBuffer {
public:
	CommandBuffer(PoolType pool);

	void submit(bool wait = true);
	void setImageLayout(
		vk::Image image,
		vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout,
		vk::ImageSubresourceRange subresourceRange,
		vk::PipelineStageFlags srcMask = vk::PipelineStageFlagBits::eAllCommands,
		vk::PipelineStageFlags dstMask = vk::PipelineStageFlagBits::eAllCommands);

	vk::CommandBuffer& get();
private:
	vk::UniqueCommandBuffer m_commandBuffer;
	vk::Device& m_logicalDevice;

	PoolType m_type;
};

