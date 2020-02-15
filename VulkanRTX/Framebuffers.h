#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>

class Device;
class Pipeline;
class Swapchain;

class Framebuffers {
public:
    Framebuffers(vk::Device& device, Swapchain& swapchain, Pipeline& pipeline);
    ~Framebuffers();

private:
    std::vector<vk::Framebuffer> framebuffers;

    vk::Device& device;
};
