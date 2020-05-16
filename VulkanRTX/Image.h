#pragma once

#include "MemoryAllocator.h"
#include "VulkanInclude.h"

class Image {
public:
	Image(const vk::Device& device,
		const uint32_t width,
		const uint32_t height,
		const vk::Format format,
		const vk::ImageUsageFlags usageFlags,
		const vk::ImageAspectFlags aspectFlags,
		const vk::ImageLayout startLayout = vk::ImageLayout::eUndefined);

	vk::Image& get();
	vk::ImageView& getView();

private:
	vk::UniqueImage m_image;
	std::unique_ptr<AllocId> m_allocId;
	vk::UniqueImageView m_imageView;
};

