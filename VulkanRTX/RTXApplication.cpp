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
    pipelineLayout = logicalDevice->createPipelineLayout();
    graphicsCommandPool = logicalDevice->createCommandPool(logicalDevice->getGraphicsQueueIndex());
    transferCommandPool = logicalDevice->createCommandPool(logicalDevice->getTransferQueueIndex());
    
    transferBuffer = logicalDevice->createCommandBuffer(*transferCommandPool);

    MemoryAllocator::init(&physicalDevice->get(), &logicalDevice->get());

    std::vector<vk::BufferCopy> bufferCopies(2);

    const std::vector<float> vertices = {
        -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 1.0f, 1.0f, 1.0f
    };

    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    std::unique_ptr<Buffer> hostIndexBuffer;
    stageDataUploadToGPU(hostIndexBuffer, indexBuffer,
                         bufferCopies[1], vk::BufferUsageFlagBits::eIndexBuffer,
                         indices);

    std::unique_ptr<Buffer> hostVertexBuffer;
    stageDataUploadToGPU(hostVertexBuffer, vertexBuffer,
                         bufferCopies[0], vk::BufferUsageFlagBits::eVertexBuffer,
                         vertices);

    std::vector<vk::Buffer*> srcBuffers;
    srcBuffers.push_back(hostVertexBuffer->getPtr());
    srcBuffers.push_back(hostIndexBuffer->getPtr());

    std::vector<vk::Buffer*> dstBuffers;
    dstBuffers.push_back(vertexBuffer->getPtr());
    dstBuffers.push_back(indexBuffer->getPtr());

    transferBuffer->begin();
    Buffer::copyBuffersToGPU(transferBuffer->get(), srcBuffers, dstBuffers, bufferCopies);
    transferBuffer->get().end();

    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &transferBuffer->get();

    vk::FenceCreateInfo fenceInfo;
    vk::UniqueFence uploadFence = logicalDevice->get().createFenceUnique(fenceInfo);

    logicalDevice->getTransferQueue().submit(submitInfo, *uploadFence);

    logicalDevice->get().waitForFences(*uploadFence, true, UINT64_MAX);

    createSwapchainHierarchy();

    size_t imageCount = swapchain->getImageViews().size();
    imagesInFlight.resize(imageCount);

    vk::SemaphoreCreateInfo semaphoreInfo;
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        imageAvailableSemaphores.push_back(logicalDevice->get().createSemaphoreUnique(semaphoreInfo));
        renderFinishedSemaphores.push_back(logicalDevice->get().createSemaphoreUnique(semaphoreInfo));
        inFlightFences.push_back(logicalDevice->get().createFenceUnique(fenceInfo));
    }
}

void RTXApplication::createSwapchainHierarchy() {
    swapchain = logicalDevice->createSwapchain(*physicalDevice, *surface);
    renderPass = logicalDevice->createRenderPass(swapchain->getFormat());
    pipeline = logicalDevice->createPipeline(*pipelineLayout, *renderPass, *swapchain);
    framebuffers = logicalDevice->createFramebuffers(*renderPass, *swapchain);

    size_t imageCount = swapchain->getImageViews().size();

    commandBuffers.resize(imageCount);

    for (size_t i = 0; i < imageCount; ++i) {
        commandBuffers[i] = logicalDevice->createCommandBuffer(*graphicsCommandPool);
        commandBuffers[i]->begin();
        commandBuffers[i]->beginRenderPass(renderPass->get(), framebuffers->getNext(), *swapchain);
        commandBuffers[i]->get().bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->get());

        vk::DeviceSize offset(0);
        commandBuffers[i]->get().bindVertexBuffers(0, vertexBuffer->get(), offset);
        commandBuffers[i]->get().bindIndexBuffer(indexBuffer->get(), 0, vk::IndexType::eUint16);
        commandBuffers[i]->get().drawIndexed(6, 1, 0, 0, 0);
        commandBuffers[i]->get().endRenderPass();
        commandBuffers[i]->get().end();
    }
}

void RTXApplication::deleteSwapchainHierarchy() {
    for (size_t i = 0; i < swapchain->getImageViews().size(); ++i) {
        commandBuffers[i].reset();
    }

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

    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        logicalDevice->get().waitForFences(1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }

    imagesInFlight[imageIndex] = *inFlightFences[currentFrame];

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
    MemoryAllocator::deinit();

    glfwDestroyWindow(window);

    glfwTerminate();
}