#include "CommandPools.h"

#include "VulkanContext.h"

void CommandPools::init(std::shared_ptr<CommandPools>& storage) {
	storage = std::make_shared<CommandPools>();
	instance = storage;

	std::shared_ptr<VulkanContext> context = VulkanContext::get();

	vk::CommandPoolCreateInfo poolInfo;
	poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

	poolInfo.queueFamilyIndex = context->m_graphicsQueueIndex;
	storage->m_graphicsCmdPool = context->m_logicalDevice->createCommandPoolUnique(poolInfo);
	storage->m_pools[PoolType::eGraphics] = &*storage->m_graphicsCmdPool;
	storage->m_queues[PoolType::eGraphics] = &context->m_graphicsQueue;

	poolInfo.queueFamilyIndex = context->m_computeQueueIndex;
	storage->m_computeCmdPool = context->m_logicalDevice->createCommandPoolUnique(poolInfo);
	storage->m_pools[PoolType::eCompute] = &*storage->m_computeCmdPool;
	storage->m_queues[PoolType::eCompute] = &context->m_computeQueue;

	poolInfo.queueFamilyIndex = context->m_transferQueueIndex;
	storage->m_transferCmdPool = context->m_logicalDevice->createCommandPoolUnique(poolInfo);
	storage->m_pools[PoolType::eTransfer] = &*storage->m_transferCmdPool;
	storage->m_queues[PoolType::eTransfer] = &context->m_transferQueue;
}

vk::CommandPool& CommandPools::getPool(PoolType type) {
	return *instance.lock()->m_pools[type];
}


vk::Queue& CommandPools::getQueue(PoolType type) {
	return *instance.lock()->m_queues[type];
}

std::weak_ptr<CommandPools> CommandPools::instance;