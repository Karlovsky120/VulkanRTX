#pragma once

#include "VulkanInclude.h"

#include "VulkanContext.h"

#include <map>

class CmdBufferAllocator {
public:
	static void init(VulkanContext* vkCtx);
	static CmdBufferAllocator* get();

	vk::UniqueCommandBuffer getTransferBuffer();
	std::vector<vk::UniqueCommandBuffer> getTransferBuffers(const uint32_t count);

	vk::UniqueCommandBuffer getGraphicsBuffer();
	std::vector<vk::UniqueCommandBuffer> getGraphicsBuffers(const uint32_t count);

	void submitBuffer(vk::CommandBuffer& cmdBuffer, bool wait = true);

private:
	CmdBufferAllocator(VulkanContext* vkCtx);

	vk::UniqueCommandPool createCommandPool(const uint32_t queueFamilyIndex) const;

	vk::UniqueCommandBuffer createBuffer(const vk::CommandPool& commandPool);
	std::vector<vk::UniqueCommandBuffer> createBuffers(
		const vk::CommandPool& commandPool,
		const uint32_t count);

	VulkanContext* m_context;

	vk::UniqueCommandPool m_transferCmdPool;
	vk::UniqueCommandPool m_graphicsCmdPool;

	std::map<const vk::CommandPool*, const vk::Queue*> poolToQueue;
	std::map<const vk::CommandBuffer, const vk::Queue*> bufferToQueue;

	static CmdBufferAllocator* instance;
};

