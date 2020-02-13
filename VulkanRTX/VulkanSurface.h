#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

class VulkanSurface {
public:
    vk::SurfaceKHR& getSurface();

    VulkanSurface(vk::Instance& instance, GLFWwindow* window);

    ~VulkanSurface();

private:
    vk::SurfaceKHR surface;

    vk::Instance& instance;
};
