#include "Image.h"

#include "VulkanContext.h"
#include "CommandBuffer.h"

Image::Image(const uint32_t width,
	const uint32_t height,
	const vk::Format format,
	const std::string name,
	const vk::ImageUsageFlags usageFlags,
	const vk::ImageAspectFlags aspectFlags,
	const vk::ImageLayout startLayout) : 
	m_imageSize(width, height) {

	vk::ImageCreateInfo imageInfo;
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = vk::SampleCountFlagBits::e1;
	imageInfo.tiling = vk::ImageTiling::eOptimal;
	imageInfo.format = format;
	imageInfo.usage = usageFlags;
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;

	m_image = VulkanContext::getDevice().createImageUnique(imageInfo);

	NAME_OBJECT(&*m_image, vk::DebugReportObjectTypeEXT::eImage, name)

	vk::MemoryRequirements memoryRequirements =
		VulkanContext::getDevice().getImageMemoryRequirements(*m_image);
	m_allocId =	MemoryAllocator::allocate(
			memoryRequirements,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
	VulkanContext::getDevice().bindImageMemory(
		*m_image,
		m_allocId->memory(),
		m_allocId->offset);

	vk::ImageSubresourceRange subresourceRange;
	subresourceRange.aspectMask = aspectFlags;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = 1;

	vk::ImageViewCreateInfo imageViewInfo;
	imageViewInfo.image = *m_image;
	imageViewInfo.viewType = vk::ImageViewType::e2D;
	imageViewInfo.format = format;
	imageViewInfo.subresourceRange = subresourceRange;

	m_imageView = VulkanContext::getDevice().createImageViewUnique(imageViewInfo);

	if (startLayout != vk::ImageLayout::eUndefined)	{
		std::unique_ptr<CommandBuffer> cmdBuffer = std::make_unique<CommandBuffer>(PoolType::eTransfer);
		cmdBuffer->setImageLayout(
			*m_image,
			vk::ImageLayout::eUndefined,
			startLayout);
		cmdBuffer->submitAndWait();
	}
}

const vk::Image& Image::get() {
	return *m_image;
}

const vk::ImageView& Image::getView() {
	return *m_imageView;
}
