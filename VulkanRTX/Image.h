#pragma once

#include "GlobalDefines.h"

#include "MemoryAllocator.h"
#include "VulkanInclude.h"

#include <vector>

class Image {
public:
	Image(const uint32_t width,
		const uint32_t height,
		const vk::Format format,
		const std::string name,
		const vk::ImageUsageFlags usageFlags,
		const vk::ImageAspectFlags aspectFlags,
		const vk::ImageLayout startLayout = vk::ImageLayout::eUndefined);

	const vk::Image& get();
	const vk::ImageView& getView();

	vk::Extent2D m_imageSize;

private:
	vk::UniqueImage m_image;
	std::unique_ptr<AllocId> m_allocId;
	vk::UniqueImageView m_imageView;
};

