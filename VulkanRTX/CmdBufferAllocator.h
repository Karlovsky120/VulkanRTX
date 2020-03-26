#pragma once

#include "VulkanInclude.h"

#include "VulkanContext.h"

#include <map>

class CmdBufferAllocator {
public:
	static void init(std::shared_ptr<CmdBufferAllocator>& storage);
	static std::shared_ptr<CmdBufferAllocator> get();

	vk::CommandBuffer createBuffer(const vk::CommandPool& commandPool);
	std::vector<vk::CommandBuffer> createBuffers(
		const vk::CommandPool& commandPool,
		const uint32_t count);

	vk::UniqueCommandBuffer createBufferUnique(const vk::CommandPool& commandPool);
	std::vector<vk::UniqueCommandBuffer> createBuffersUnique(
		const vk::CommandPool& commandPool,
		const uint32_t count);

	void submitBuffer(vk::CommandBuffer& cmdBuffer, vk::Queue& queue, bool wait = true);

	vk::UniqueCommandPool m_graphicsCmdPool;
	vk::UniqueCommandPool m_computeCmdPool;
	vk::UniqueCommandPool m_transferCmdPool;

	CmdBufferAllocator(CmdBufferAllocator const&) = delete;
	void operator=(CmdBufferAllocator const&) = delete;

	CmdBufferAllocator();

private:

	vk::UniqueCommandPool createCommandPool(const uint32_t queueFamilyIndex) const;

	std::weak_ptr<VulkanContext> m_context;

	static std::weak_ptr<CmdBufferAllocator> instance;
};

