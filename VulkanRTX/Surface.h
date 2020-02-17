#pragma once

#include <vulkan/vulkan.hpp>

class Instance;

struct GLFWwindow;

class Surface {
public:
    vk::SurfaceKHR& get();

    uint32_t getWidth();
    uint32_t getHeight();
    GLFWwindow* getWindow();

    Surface(Instance& instance, GLFWwindow* window, uint32_t width, uint32_t height);
    ~Surface();

private:
    vk::SurfaceKHR m_surface;

    uint32_t m_width;
    uint32_t m_height;
    GLFWwindow* m_window;

    Instance& m_instance;
};
