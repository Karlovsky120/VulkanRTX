#pragma once

#include "VulkanInclude.h"

#include <map>

enum class PoolType {
	eGraphics,
	eCompute,
	eTransfer
};

class CommandPools {
public:
	static void init(std::shared_ptr<CommandPools>& storage);
	static vk::CommandPool& getPool(PoolType type);
	static vk::Queue& getQueue(PoolType type);

private:
	static std::weak_ptr<CommandPools> instance;

	std::map<PoolType, vk::CommandPool*> m_pools;
	std::map<PoolType, vk::Queue*> m_queues;

	vk::UniqueCommandPool m_graphicsCmdPool;
	vk::UniqueCommandPool m_computeCmdPool;
	vk::UniqueCommandPool m_transferCmdPool;
};

