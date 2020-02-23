#pragma once

#include <vulkan/vulkan.hpp>

class Instance;

struct GLFWwindow;

class Surface {
public:
    vk::SurfaceKHR& get();

    uint32_t getWidth() const;
    uint32_t getHeight() const;
    GLFWwindow* getWindow();

    Surface(vk::Instance& instance,
            GLFWwindow* window,
            const uint32_t width,
            const uint32_t height);
    ~Surface();

private:
    vk::SurfaceKHR m_surface;

    const uint32_t m_width;
    const uint32_t m_height;
    GLFWwindow* m_window;

    vk::Instance& m_instance;
};
