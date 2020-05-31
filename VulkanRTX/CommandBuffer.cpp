#include "CommandBuffer.h"

#include "Image.h"
#include "VulkanContext.h"

#ifdef AFTERMATH
#include <chrono>
#include <thread>
#endif

CommandBuffer::CommandBuffer(PoolType type) :
	m_type(type) {

	reset();
}

void CommandBuffer::submit() {
	m_commandBuffer->end();

	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &*m_commandBuffer;

#ifdef AFTERMATH
	vk::Result result = CommandPools::getQueue(m_type).submit(1, &submitInfo, nullptr);
	if (result == vk::Result::eErrorDeviceLost) {
		std::this_thread::sleep_for(std::chrono::seconds(3));
		throw std::runtime_error("Error device lost");
	}
#else
	CommandPools::getQueue(m_type).submit(submitInfo, nullptr);
#endif
}

void CommandBuffer::submitAndWait() {
	m_commandBuffer->end();

	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &*m_commandBuffer;

	vk::UniqueFence fence = VulkanContext::getDevice().createFenceUnique({});
#ifdef AFTERMATH
	vk::Result result = CommandPools::getQueue(m_type).submit(1, &submitInfo, *fence);
	if (result == vk::Result::eErrorDeviceLost) {
		std::this_thread::sleep_for(std::chrono::seconds(3));
		throw std::runtime_error("Error device lost");
	}
#else
	CommandPools::getQueue(m_type).submit(submitInfo, *fence);
#endif
	VulkanContext::getDevice().waitForFences(*fence, VK_TRUE, UINT64_MAX);
}

void CommandBuffer::setImageLayout(
	const vk::Image& image,
	const vk::ImageLayout& oldLayout,
	const vk::ImageLayout& newLayout,
	const vk::ImageSubresourceRange& subresourceRange,
	const vk::PipelineStageFlags& srcMask,
	const vk::PipelineStageFlags& dstMask) {

	vk::ImageMemoryBarrier barrier;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.image = image;
	barrier.subresourceRange = subresourceRange;

	switch (oldLayout) {
	case vk::ImageLayout::eUndefined:
		break;

	case vk::ImageLayout::eTransferSrcOptimal:
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		break;

	case vk::ImageLayout::eTransferDstOptimal:
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		break;

	case vk::ImageLayout::eGeneral:
		break;

	default:
		throw std::runtime_error("Old layout not handled by setImageLayout function!");
	}

	switch (newLayout) {
	case vk::ImageLayout::eTransferSrcOptimal:
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
		break;

	case vk::ImageLayout::eTransferDstOptimal:
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		break;

	case vk::ImageLayout::eGeneral:
		break;

	case vk::ImageLayout::ePresentSrcKHR:
		break;

	default:
		throw std::runtime_error("New layout not handled by setImageLayout function!");
	}

	m_commandBuffer->pipelineBarrier(
		srcMask,
		dstMask,
		vk::DependencyFlags(),
		{},
		{},
		{ barrier });
}

void CommandBuffer::copyImageToBuffer(
	const vk::Image& image,
	const vk::Extent2D& imageSize,
	const vk::ImageLayout& oldLayout,
	const vk::Buffer& buffer) {

	setImageLayout(image, oldLayout, vk::ImageLayout::eTransferSrcOptimal);

	vk::BufferImageCopy copyRegion;
	copyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	copyRegion.imageSubresource.mipLevel = 0;
	copyRegion.imageSubresource.baseArrayLayer = 0;
	copyRegion.imageSubresource.layerCount = 1;
	copyRegion.imageExtent = vk::Extent3D(imageSize, 1);

	m_commandBuffer->copyImageToBuffer(
		image,
		vk::ImageLayout::eTransferSrcOptimal,
		buffer,
		{ copyRegion });
}

void CommandBuffer::copyBufferToImage(
	const vk::Buffer& buffer,
	const vk::Image& image,
	const vk::Extent2D& imageSize,
	const vk::ImageLayout& oldLayout) {

	setImageLayout(image, oldLayout, vk::ImageLayout::eTransferDstOptimal);

	vk::BufferImageCopy copyRegion;
	copyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	copyRegion.imageSubresource.mipLevel = 0;
	copyRegion.imageSubresource.baseArrayLayer = 0;
	copyRegion.imageSubresource.layerCount = 1;
	copyRegion.imageExtent = vk::Extent3D(imageSize, 1);

	m_commandBuffer->copyBufferToImage(
		buffer,
		image,
		vk::ImageLayout::eTransferDstOptimal,
		{ copyRegion });
}

void CommandBuffer::reset() {
	m_commandBuffer.reset();

	vk::CommandBufferAllocateInfo commandBufferAllocateInfo;
	commandBufferAllocateInfo.commandPool = CommandPools::getPool(m_type);
	commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
	commandBufferAllocateInfo.commandBufferCount = 1;

	m_commandBuffer = std::move(VulkanContext::getDevice().allocateCommandBuffersUnique(
		commandBufferAllocateInfo)[0]);

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	m_commandBuffer->begin(beginInfo);
}

vk::CommandBuffer& CommandBuffer::get() {
	return *m_commandBuffer;
}

PoolType CommandBuffer::getPoolType() {
	return m_type;
}