#pragma once

#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanSurface.h"

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <memory>

class RTXApplication {
public:
    void run();

    ~RTXApplication();

private:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    const uint32_t WIDTH = 1280;
    const uint32_t HEIGHT = 720;

    GLFWwindow* window;

    std::unique_ptr<VulkanInstance> instance;
    std::unique_ptr<VulkanSurface> surface;
    std::unique_ptr<VulkanDevice> device;

    void initVulkan();
    void initWindow();
    void mainLoop();
};
