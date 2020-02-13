#include "VulkanSurface.h"

VulkanSurface::VulkanSurface(vk::Instance& instance, GLFWwindow* window) :
    instance(instance) {

    VkSurfaceKHR vulkanSurface;
    if (glfwCreateWindowSurface(instance, window, nullptr, &vulkanSurface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }

    surface = vk::SurfaceKHR(vulkanSurface);
}

VulkanSurface::~VulkanSurface() {
    instance.destroySurfaceKHR(surface);
}