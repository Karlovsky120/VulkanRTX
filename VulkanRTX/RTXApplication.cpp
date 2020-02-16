#include "RTXApplication.h"

#include <GLFW/glfw3.h>

void RTXApplication::run() {
    initWindow();
    initVulkan();
    mainLoop();
}

void RTXApplication::initVulkan() {
    instance = std::make_unique<Instance>();
    surface = instance->createSurface(window, WIDTH, HEIGHT);
    physicalDevice = instance->createPhysicalDevice();
    logicalDevice = physicalDevice->createLogicalDevice(*surface);
    swapchain = logicalDevice->createSwapchain(*physicalDevice, *surface);
    renderPass = logicalDevice->createRenderPass(*swapchain);
    pipeline = logicalDevice->createPipeline(*swapchain);
    framebuffers = logicalDevice->createFramebuffers(*swapchain, *renderPass);
    commandPool = logicalDevice->createCommandPool(logicalDevice->getGraphicsQueueIndex());
    commandBuffer = logicalDevice->createCommandBuffer(*commandPool);
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