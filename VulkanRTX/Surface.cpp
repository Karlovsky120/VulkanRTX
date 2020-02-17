#include "Surface.h"

#include "Instance.h"

#include <GLFW/glfw3.h>

vk::SurfaceKHR& Surface::get() {
    return m_surface;
}

uint32_t Surface::getWidth() {
    return m_width;
}

uint32_t Surface::getHeight() {
    return m_height;
}

GLFWwindow* Surface::getWindow() {
    return m_window;
}

Surface::Surface(Instance& instance, GLFWwindow* window, uint32_t width, uint32_t height) :
    m_instance(instance),
    m_window(window),
    m_width(width),
    m_height(height) {

    VkSurfaceKHR vulkanSurface;
    if (glfwCreateWindowSurface(instance.get(), window, nullptr, &vulkanSurface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }

    m_surface = vk::SurfaceKHR(vulkanSurface);
}

Surface::~Surface() {
    m_instance.get().destroySurfaceKHR(m_surface);
}