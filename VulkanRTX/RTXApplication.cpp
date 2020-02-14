#include "RTXApplication.h"

#include <GLFW/glfw3.h>

void RTXApplication::run() {
    initWindow();
    initVulkan();
    mainLoop();
}

void RTXApplication::initVulkan() {
    instance = std::make_unique<VulkanInstance>(true);
    surface = std::make_unique<VulkanSurface>(instance->getInstance(), window);
    device = std::make_unique<VulkanDevice>(instance->getInstance(), surface->getSurface());
    swapchain = std::make_unique<VulkanSwapchain>(*device, surface->getSurface(), WIDTH, HEIGHT);
    pipeline = std::make_unique<VulkanPipeline>(device->getDevice(), *swapchain);
}

void RTXApplication::initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan shenanigans", nullptr, nullptr);
}

void RTXApplication::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

RTXApplication::~RTXApplication() {
    glfwDestroyWindow(window);

    glfwTerminate();
}