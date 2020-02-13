#pragma once

#include <vulkan/vulkan.hpp>

class GLFWwindow;

class VulkanSurface {
public:
    vk::SurfaceKHR& getSurface();

    VulkanSurface(vk::Instance& instance, GLFWwindow* window);

    ~VulkanSurface();

private:
    vk::SurfaceKHR surface;

    vk::Instance& instance;
};
