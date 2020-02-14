#pragma once

#include "Device.h"
#include "Instance.h"
#include "Pipeline.h"
#include "Surface.h"
#include "Swapchain.h"

#include <vulkan/vulkan.hpp>

#include <memory>

struct GLFWwindow;

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

    std::unique_ptr<Instance> instance;
    std::unique_ptr<Surface> surface;
    std::unique_ptr<Device> device;
    std::unique_ptr<Swapchain> swapchain;
    std::unique_ptr<Pipeline> pipeline;

    void initVulkan();
    void initWindow();
    void mainLoop();
};
