#include "RTXApplication.h"

#include <GLFW/glfw3.h>

void RTXApplication::run() {
    initWindow();
    initVulkan();
    mainLoop();
}

void RTXApplication::initVulkan() {
    instance = std::make_unique<Instance>(true);
    surface = std::make_unique<Surface>(instance->getInstance(), window);
    device = std::make_unique<Device>(instance->getInstance(), surface->getSurface());
    swapchain = std::make_unique<Swapchain>(*device, surface->getSurface(), WIDTH, HEIGHT);
    pipeline = std::make_unique<Pipeline>(device->getDevice(), *swapchain);
    framebuffers = std::make_unique<Framebuffers>(device->getDevice(), *swapchain, *pipeline);
    renderPass = std::make_unique<RenderPass>(device->getDevice(), *swapchain);
    commandPool = std::make_unique<CommandPool>(*device, device->getGraphicsQueueIndex());
    commandBuffer = std::make_unique<CommandBuffer>(device->getDevice(), commandPool->getCommandPool());
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