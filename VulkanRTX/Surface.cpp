#include "Surface.h"

#include "Instance.h"

#include <GLFW/glfw3.h>

vk::SurfaceKHR& Surface::get() {
    return m_surface;
}

uint32_t Surface::getWidth() const {
    return m_width;
}

uint32_t Surface::getHeight() const {
    return m_height;
}

GLFWwindow* Surface::getWindow() {
    return m_window;
}

Surface::Surface(vk::Instance& instance, GLFWwindow* window, const uint32_t width, const uint32_t height) :
    m_instance(instance),
    m_window(window),
    m_width(width),
    m_height(height) {

    VkSurfaceKHR vulkanSurface;
    if (glfwCreateWindowSurface(instance, window, nullptr, &vulkanSurface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }

    m_surface = vk::SurfaceKHR(vulkanSurface);
}

Surface::~Surface() {
    m_instance.destroySurfaceKHR(m_surface);
}