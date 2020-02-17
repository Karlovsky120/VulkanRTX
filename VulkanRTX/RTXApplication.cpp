#include "RTXApplication.h"

#include <GLFW/glfw3.h>

#include <iostream>

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
    commandPool = logicalDevice->createCommandPool(logicalDevice->getGraphicsQueueIndex());

    createSwapchainHierarchy();

    vk::SemaphoreCreateInfo semaphoreInfo;
    vk::FenceCreateInfo fenceInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    size_t imageCount = swapchain->getImageViews().size();

    imagesInFlight.resize(imageCount, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        imageAvailableSemaphores.push_back(logicalDevice->get().createSemaphoreUnique(semaphoreInfo));
        renderFinishedSemaphores.push_back(logicalDevice->get().createSemaphoreUnique(semaphoreInfo));
        inFlightFences.push_back(logicalDevice->get().createFenceUnique(fenceInfo));
    }
}

void RTXApplication::createSwapchainHierarchy() {
    swapchain = logicalDevice->createSwapchain(*physicalDevice, *surface);
    renderPass = logicalDevice->createRenderPass(*swapchain);
    pipeline = logicalDevice->createPipeline(*renderPass, *swapchain);
    framebuffers = logicalDevice->createFramebuffers(*swapchain, *renderPass);

    size_t imageCount = swapchain->getImageViews().size();

    for (size_t i = 0; i < imageCount; ++i) {
        commandBuffers.push_back(logicalDevice->createCommandBuffer(*commandPool));
        commandBuffers[i]->begin();
        commandBuffers[i]->beginRenderPass(renderPass->get(), framebuffers->getNext(), *swapchain);
        commandBuffers[i]->get().bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->get());
        commandBuffers[i]->get().draw(3, 1, 0, 0);
        commandBuffers[i]->get().endRenderPass();
        commandBuffers[i]->get().end();
    }
}

void RTXApplication::deleteSwapchainHierarchy() {
    for (size_t i = 0; i < swapchain->getImageViews().size(); ++i) {
        commandBuffers[i].reset();
    }

    commandBuffers.clear();
    framebuffers.reset();
    pipeline.reset();
    renderPass.reset();
    swapchain.reset();
}

void RTXApplication::recreateSwapchainHierarchy() {
    int width = 0;
    int height = 0;

    glfwGetFramebufferSize(window, &width, &height);

    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    logicalDevice->get().waitIdle();

    deleteSwapchainHierarchy();
    createSwapchainHierarchy();
}

void RTXApplication::initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan shenanigans", nullptr, nullptr);

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
}

void RTXApplication::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<RTXApplication*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

void RTXApplication::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        drawFrame();
    }

    logicalDevice->get().waitIdle();
}

void RTXApplication::drawFrame() {

    // Render
    logicalDevice->get().waitForFences(1, &*inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;

    vk::Result acquireResult;

    try {
        acquireResult = logicalDevice->get().acquireNextImageKHR(swapchain->get(), UINT64_MAX, *imageAvailableSemaphores[currentFrame], nullptr, &imageIndex);
    }
    catch (vk::OutOfDateKHRError) {
        recreateSwapchainHierarchy();
        return;
    }

    if (acquireResult != vk::Result::eSuccess && acquireResult != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Failed to acquire swapchain image!");
    }

    if (imagesInFlight[imageIndex] != nullptr) {
        logicalDevice->get().waitForFences(1, &**imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }

    imagesInFlight[imageIndex] = &inFlightFences[currentFrame];

    vk::SubmitInfo submitInfo;
    submitInfo.waitSemaphoreCount = 1;

    vk::Semaphore waitSemaphores[] = {*imageAvailableSemaphores[currentFrame]};
    submitInfo.pWaitSemaphores = waitSemaphores;

    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex]->get();

    vk::Semaphore signalSemaphores[] = {*renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    logicalDevice->get().resetFences(1, &*inFlightFences[currentFrame]);

    logicalDevice->getGraphicsQueue().submit(submitInfo, *inFlightFences[currentFrame]);

    // Present
    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    vk::SwapchainKHR swapchains[] = {swapchain->get()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;

    vk::Result presentResult;
    try {
        presentResult = logicalDevice->getPresentQueue().presentKHR(presentInfo);
    }
    catch (vk::OutOfDateKHRError) {
        presentResult = vk::Result::eErrorOutOfDateKHR;
    }

    if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapchainHierarchy();
    }
    else if (acquireResult != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to acquire swapchain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

RTXApplication::~RTXApplication() {
    glfwDestroyWindow(window);

    glfwTerminate();
}