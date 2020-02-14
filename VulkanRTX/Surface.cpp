#include "Surface.h"

#include <GLFW/glfw3.h>

vk::SurfaceKHR& Surface::getSurface() {
    return surface;
}

Surface::Surface(vk::Instance& instance, GLFWwindow* window) :
    instance(instance) {

    VkSurfaceKHR vulkanSurface;
    if (glfwCreateWindowSurface(instance, window, nullptr, &vulkanSurface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }

    surface = vk::SurfaceKHR(vulkanSurface);
}

Surface::~Surface() {
    instance.destroySurfaceKHR(surface);
}