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

    vk::SemaphoreCreateInfo semaphoreInfo;
    vk::FenceCreateInfo fenceInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    imagesInFlight.resize(swapchain->getImageViews().size(), nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        imageAvailableSemaphores.push_back(logicalDevice->get().createSemaphoreUnique(semaphoreInfo));
        renderFinishedSemaphores.push_back(logicalDevice->get().createSemaphoreUnique(semaphoreInfo));
        inFlightFences.push_back(logicalDevice->get().createFenceUnique(fenceInfo));
    }
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
        drawFrame();
    }

    logicalDevice->get().waitIdle();
}

void RTXApplication::drawFrame() {
    logicalDevice->get().waitForFences(1, &*inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;

    logicalDevice->get().acquireNextImageKHR(swapchain->get(), UINT64_MAX, *imageAvailableSemaphores[currentFrame], nullptr, &imageIndex);

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

    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    vk::SwapchainKHR swapchains[] = {swapchain->get()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;

    logicalDevice->getPresentQueue().presentKHR(presentInfo);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

RTXApplication::~RTXApplication() {
    glfwDestroyWindow(window);

    glfwTerminate();
}