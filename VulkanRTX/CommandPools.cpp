#include "CommandPools.h"

#include "VulkanContext.h"

void CommandPools::init(std::shared_ptr<CommandPools>& storage) {
	storage = std::make_shared<CommandPools>();
	instance = storage;

	std::weak_ptr<VulkanContext> context = VulkanContext::get();

	vk::CommandPoolCreateInfo poolInfo;
	poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

	poolInfo.queueFamilyIndex = context.lock()->m_graphicsQueueIndex;
	storage->m_graphicsCmdPool = context.lock()->m_logicalDevice->createCommandPoolUnique(poolInfo);
	storage->m_pools[PoolType::eGraphics] = &*storage->m_graphicsCmdPool;
	storage->m_queues[PoolType::eGraphics] = &context.lock()->m_graphicsQueue;

	poolInfo.queueFamilyIndex = context.lock()->m_computeQueueIndex;
	storage->m_computeCmdPool = context.lock()->m_logicalDevice->createCommandPoolUnique(poolInfo);
	storage->m_pools[PoolType::eCompute] = &*storage->m_computeCmdPool;
	storage->m_queues[PoolType::eCompute] = &context.lock()->m_computeQueue;

	poolInfo.queueFamilyIndex = context.lock()->m_transferQueueIndex;
	storage->m_transferCmdPool = context.lock()->m_logicalDevice->createCommandPoolUnique(poolInfo);
	storage->m_pools[PoolType::eTransfer] = &*storage->m_transferCmdPool;
	storage->m_queues[PoolType::eTransfer] = &context.lock()->m_transferQueue;
}

vk::CommandPool& CommandPools::getPool(PoolType type) {
	return *instance.lock()->m_pools[type];
}


vk::Queue& CommandPools::getQueue(PoolType type) {
	return *instance.lock()->m_queues[type];
}

std::weak_ptr<CommandPools> CommandPools::instance;