#pragma once

#include "GlobalDefines.h"

#include "CommandPools.h"
#include "VulkanInclude.h"

class Image;
class VulkanContext;

class CommandBuffer {
public:
	CommandBuffer(PoolType pool);

	void submit();
	void submitAndWait();
	void reset();

	void setImageLayout(
		const vk::Image& image,
		const vk::ImageLayout& oldLayout,
		const vk::ImageLayout& newLayout,
		const vk::ImageSubresourceRange& subresourceRange =
		{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 },
		const vk::PipelineStageFlags& srcMask = vk::PipelineStageFlagBits::eAllCommands,
		const vk::PipelineStageFlags& dstMask = vk::PipelineStageFlagBits::eAllCommands);

	void copyImageToBuffer(
		const vk::Image& image,
		const vk::Extent2D& imageSize,
		const vk::ImageLayout& oldLayout,
		const vk::Buffer& buffer);

	void copyBufferToImage(
		const vk::Buffer& buffer,
		const vk::Image& image,
		const vk::Extent2D& imageSize,
		const vk::ImageLayout& oldLayout);

	vk::CommandBuffer& get();
	PoolType getPoolType();

private:
	vk::UniqueCommandBuffer m_commandBuffer;

	PoolType m_type;
};

