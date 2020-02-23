#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>

class Swapchain;

class Framebuffers {
public:
    vk::Framebuffer& get(const uint32_t index);
    vk::Framebuffer& getNext();

    Framebuffers(vk::Device& device, vk::RenderPass& renderPass, Swapchain& swapchain);

private:
    std::vector<vk::UniqueFramebuffer> m_framebuffers;

    uint32_t m_currentIndex = 0;

    vk::Device& m_logicalDevice;
};
