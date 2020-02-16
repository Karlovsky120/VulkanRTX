#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>

class LogicalDevice;
class RenderPass;
class Swapchain;

class Framebuffers {
public:
    vk::Framebuffer& get(uint32_t index);
    vk::Framebuffer& getNext();

    Framebuffers(LogicalDevice& device, Swapchain& swapchain, RenderPass& renderPass);
    ~Framebuffers();

private:
    std::vector<vk::Framebuffer> m_framebuffers;

    uint32_t m_currentIndex = 0;

    LogicalDevice& m_logicalDevice;
};
