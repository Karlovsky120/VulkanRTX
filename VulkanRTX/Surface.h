#pragma once

#include <vulkan/vulkan.hpp>

struct GLFWwindow;

class Surface {
public:
    vk::SurfaceKHR& getSurface();

    Surface(vk::Instance& instance, GLFWwindow* window);

    ~Surface();

private:
    vk::SurfaceKHR surface;

    vk::Instance& instance;
};
