#include "RTXApplication.h"

#include "Buffer.h"
#include "CmdBufferAllocator.h"
#include "MemoryAllocator.h"
#include "RayTracing.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <chrono>
#include <iostream>

void RTXApplication::run() {
    initWindow();
    initVulkan();
    initOther();
    mainLoop();
}

void RTXApplication::initVulkan() {

    VulkanContext::init(vkCtx, window);
    MemoryAllocator::init(
        memoryAllocator,
        *vkCtx->m_logicalDevice,
        vkCtx->m_deviceMemoryProperties);
    CmdBufferAllocator::init(cmdBufferAllocator);

    swapchain = std::make_unique<Swapchain>(
        vkCtx->m_physicalDevice,
        *vkCtx->m_surface,
        *vkCtx->m_logicalDevice,
        vkCtx->m_presentQueue);

    descriptorSetLayout = vkCtx->createUniformDescriptorSetLayout();

    pipeline = std::make_unique<Pipeline>(*vkCtx->m_logicalDevice);
    pipeline->initPipeline(swapchain->m_extent, swapchain->m_colorFormat, &*descriptorSetLayout);

    swapchain->updateFramebuffers(*pipeline->m_renderPass);

    vk::UniqueCommandBuffer transferCmdBuffer =
        CmdBufferAllocator::get()->createBufferUnique(*CmdBufferAllocator::get()->m_transferCmdPool);

    const std::vector<float> vertices = {
        -1, -1, -1, 0, 0, 1,
        1, -1, -1, 0, 1, 0,
        1, 1, -1, 1, 0, 0,
        -1, 1, -1, 0, 1, 1,
        -1, -1, 1, 1, 1, 0,
        1, -1, 1, 1, 0, 1,
        1, 1, 1, 0, 0, 0,
        -1, 1, 1, 1, 1, 1
    };

    const std::vector<uint16_t> indices = {
        0, 1, 3, 3, 1, 2,
        1, 5, 2, 2, 5, 6,
        5, 4, 6, 6, 4, 7,
        4, 0, 7, 7, 0, 3,
        3, 2, 7, 7, 2, 6,
        4, 5, 0, 0, 5, 1
    };

    object = std::make_unique<Mesh>(*vkCtx->m_logicalDevice, vertices, indices);
    object->translate(glm::vec3(0.0f, 0.0f, 2.5f));

    uniformBuffer = std::make_unique<Buffer>(
        *vkCtx->m_logicalDevice,
        sizeof(UniformBufferObject),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    descriptorPool = vkCtx->createDescriptorPool(swapchain->m_imageCount);

    swapchainFrameInfos.resize(swapchain->m_imageCount);
    
    std::vector<vk::DescriptorSetLayout> layouts(swapchain->m_imageCount, *descriptorSetLayout);
    std::vector<vk::UniqueDescriptorSet> descriptorSets = vkCtx->createDescriptorSets(*descriptorPool, layouts);
    std::vector<vk::CommandBuffer> cmdBuffers =
        CmdBufferAllocator::get()->createBuffers(*CmdBufferAllocator::get()->m_graphicsCmdPool, static_cast<uint32_t>(swapchainFrameInfos.size()));

    for (uint32_t i = 0; i < swapchainFrameInfos.size(); ++i) {
        swapchainFrameInfos[i].descriptorSet = std::move(descriptorSets[i]);
        swapchainFrameInfos[i].frameBuffer = std::move(cmdBuffers[i]);

        generateSwapchainFrameInfo(i);
        updateSwapchainFrameInfo(i);
    }

    inFlightFrameInfos.resize(MAX_FRAMES_IN_FLIGHT);
    vk::SemaphoreCreateInfo semaphoreCreateInfo;
    vk::FenceCreateInfo fenceCreateInfo;
    fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        inFlightFrameInfos[i].imageAvailableSemaphore = vkCtx->m_logicalDevice->createSemaphoreUnique(semaphoreCreateInfo);
        inFlightFrameInfos[i].renderCompleteSemaphore = vkCtx->m_logicalDevice->createSemaphoreUnique(semaphoreCreateInfo);
        inFlightFrameInfos[i].inFlightFence = vkCtx->m_logicalDevice->createFenceUnique(fenceCreateInfo);
    }

    glfwGetCursorPos(window, &cursorX, &cursorY);
}

void RTXApplication::initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(windowWidth, windowHeight, "Vulkan shenanigans", nullptr, nullptr);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
}

void RTXApplication::initOther() {
    camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f), 90.0f, windowWidth / (float)windowHeight, 0.1f, 100.0f);
}

void RTXApplication::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        calculateTime();
        updateFPS();
        processMouse();
        processKeyboard();

        uint32_t swapchainIndex = acquireNextImage();

        if (swapchainIndex != -1) {
            updateUniformBuffer();
            resetCommandBuffer(swapchainIndex);
            recordCommandBuffer(swapchainIndex);
            drawFrame(swapchainIndex);
        }
    }

    vkCtx->m_logicalDevice->waitIdle();
}

void RTXApplication::generateSwapchainFrameInfo(const uint32_t index) {
    SwapchainFrameInfo& frameInfo = swapchainFrameInfos[index];

    frameInfo.descriptorBufferInfo.buffer = uniformBuffer->get();
    frameInfo.descriptorBufferInfo.offset = 0;
    frameInfo.descriptorBufferInfo.range = sizeof(UniformBufferObject);

    frameInfo.writeDescriptorSet.dstSet = *frameInfo.descriptorSet;
    frameInfo.writeDescriptorSet.dstBinding = 0;
    frameInfo.writeDescriptorSet.dstArrayElement = 0;
    frameInfo.writeDescriptorSet.descriptorType = vk::DescriptorType::eUniformBuffer;
    frameInfo.writeDescriptorSet.descriptorCount = 1;
    frameInfo.writeDescriptorSet.pBufferInfo = &frameInfo.descriptorBufferInfo;

    vkCtx->m_logicalDevice->updateDescriptorSets(1, &frameInfo.writeDescriptorSet, 0, nullptr);

    frameInfo.clearColor = vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f }));

    frameInfo.renderPassBeginInfo.clearValueCount = 1;
    frameInfo.renderPassBeginInfo.pClearValues = &frameInfo.clearColor;
    frameInfo.renderPassBeginInfo.renderArea.offset = vk::Offset2D(0, 0);
}

void RTXApplication::updateSwapchainStack() {

    camera.updateProjectionMatrix(windowWidth / (float)windowHeight);

    for (uint32_t i = 0; i < swapchainFrameInfos.size(); ++i) {
        resetCommandBuffer(i);
    }

    // First update the swapchain according to the surface change
    swapchain->updateSwapchain();

    // Use new values of the swapchain to update the pipeline and renderpass
    pipeline->updatePipeline(swapchain->m_colorFormat, swapchain->m_extent);

    // Use the newly created renderpass to update the framebuffers
    swapchain->updateFramebuffers(*pipeline->m_renderPass);

    // Update command buffers to use the updated objects
    for (uint32_t i = 0; i < swapchainFrameInfos.size(); ++i) {

        // Update info structures
        updateSwapchainFrameInfo(i);

        // Rerecord the command buffers
        recordCommandBuffer(i);
    }
}

void RTXApplication::updateSwapchainFrameInfo(const uint32_t index) {
    SwapchainFrameInfo& frameInfo = swapchainFrameInfos[index];

    frameInfo.renderPassBeginInfo.renderPass = *pipeline->m_renderPass;
    frameInfo.renderPassBeginInfo.framebuffer = *swapchain->m_framebuffers[index];
    frameInfo.renderPassBeginInfo.renderArea.extent = swapchain->m_extent;
}

void RTXApplication::recordCommandBuffer(const uint32_t index) {
    SwapchainFrameInfo& frameInfo = swapchainFrameInfos[index];
    
    vk::CommandBuffer& frameBuffer = frameInfo.frameBuffer;

    frameBuffer.begin(frameInfo.beginInfo);
    frameBuffer.beginRenderPass(frameInfo.renderPassBeginInfo, vk::SubpassContents::eInline);
    frameBuffer.pushConstants(
        *pipeline->m_pipelineLayout,
        vk::ShaderStageFlagBits::eVertex,
        0,
        sizeof(glm::mat4),
        glm::value_ptr(camera.getCameraMatrix()));

    frameBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline->m_pipeline);

    frameBuffer.bindVertexBuffers(0, object->getVertexBuffer(), vk::DeviceSize(0));
    frameBuffer.bindIndexBuffer(object->getIndexBuffer(), 0, vk::IndexType::eUint16);
    frameBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        *pipeline->m_pipelineLayout,
        0,
        *frameInfo.descriptorSet,
        { nullptr });

    frameBuffer.drawIndexed(36, 1, 0, 0, 0);
    frameBuffer.endRenderPass();
    frameBuffer.end();
}

void RTXApplication::resetCommandBuffer(const uint32_t index) {
    SwapchainFrameInfo& frameInfo = swapchainFrameInfos[index];

    if (frameInfo.imageInUse != VK_NULL_HANDLE) {
        vkCtx->m_logicalDevice->waitForFences(
            frameInfo.imageInUse,
            VK_TRUE,
            UINT64_MAX);
    }
    frameInfo.frameBuffer.reset(vk::CommandBufferResetFlags());
}

void RTXApplication::calculateTime() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    frameTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - time).count();

    time = currentTime;
}

void RTXApplication::updateFPS() {
    skip += frameTime;
    if (skip > 0.2f) {
        windowTitle = "Vulkan shenaningans FPS: " + std::to_string(1.0f / frameTime);
        skip = 0;
    }

    glfwSetWindowTitle(window, windowTitle.c_str());
}

void RTXApplication::processMouse() {
    double newCursorX;
    double newCursorY;
    glfwGetCursorPos(window, &newCursorX, &newCursorY);

    double deltaX = newCursorX - cursorX;
    double deltaY = newCursorY - cursorY;

    camera.rotate(0.1f * glm::radians(deltaY), -0.1f * glm::radians(deltaX));

    cursorX = newCursorX;
    cursorY = newCursorY;
}

void RTXApplication::processKeyboard() {
    glm::vec3 translateVector = glm::vec3(0.0f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        translateVector.z += 1.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        translateVector.z -= 1.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        translateVector.x -= 1.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        translateVector.x += 1.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        translateVector.y -= 1.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        translateVector.y += 1.0f;
    }

    if (glm::length2(translateVector) > 0.0000001f) {
        camera.translateOriented(glm::normalize(translateVector) * velocity * frameTime);
    }
}

uint32_t RTXApplication::acquireNextImage() {
    InFlightFrameInfo& frameInfo = inFlightFrameInfos[currentFrame];

    vkCtx->m_logicalDevice->waitForFences(*frameInfo.inFlightFence, VK_TRUE, UINT64_MAX);

    uint32_t swapchainIndex;
    vk::Result acquireResult;

    try {
        acquireResult = vkCtx->m_logicalDevice->acquireNextImageKHR(
            *swapchain->m_swapchain,
            UINT64_MAX,
            *frameInfo.imageAvailableSemaphore,
            nullptr,
            &swapchainIndex);
    }
    catch (vk::OutOfDateKHRError) {
        updateSwapchainStack();
        return -1;
    }

    if (acquireResult != vk::Result::eSuccess && acquireResult != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Failed to acquire swapchain image!");
    }

    SwapchainFrameInfo& swapchainInfo = swapchainFrameInfos[swapchainIndex];

    if (swapchainInfo.imageInUse != VK_NULL_HANDLE) {
        vkCtx->m_logicalDevice->waitForFences(swapchainInfo.imageInUse, VK_TRUE, UINT64_MAX);
    }

    swapchainInfo.imageInUse = *frameInfo.inFlightFence;

    return swapchainIndex;
}

void RTXApplication::updateUniformBuffer() {
    object->rotate(glm::vec3(0.0f, 2 * frameTime, 0.0f));

    UniformBufferObject ubo;
    ubo.model = object->getMeshMatrix();

    uniformBuffer->uploadToHostLocal(ubo);
}

void RTXApplication::drawFrame(const uint32_t swapchainIndex) {
    SwapchainFrameInfo& swapchainFrameInfo = swapchainFrameInfos[swapchainIndex];
    InFlightFrameInfo& flightFrameInfo = inFlightFrameInfos[currentFrame];

    vk::SubmitInfo submitInfo;
    submitInfo.waitSemaphoreCount = 1;

    vk::Semaphore imageAvailable[] = { *flightFrameInfo.imageAvailableSemaphore };
    submitInfo.pWaitSemaphores = imageAvailable;

    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &swapchainFrameInfo.frameBuffer;

    vk::Semaphore renderComplete[] = { *flightFrameInfo.renderCompleteSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = renderComplete;

    vkCtx->m_logicalDevice->resetFences(1, &*flightFrameInfo.inFlightFence);
    vkCtx->m_graphicsQueue.submit(submitInfo, *flightFrameInfo.inFlightFence);

    vk::PresentInfoKHR presentInfo;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = renderComplete;

    vk::SwapchainKHR swapchains[] = { *swapchain->m_swapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &swapchainIndex;

    vk::Result presentResult;

    try {
        presentResult = vkCtx->m_presentQueue.presentKHR(presentInfo);
    }
    catch (vk::OutOfDateKHRError) {
        presentResult = vk::Result::eErrorOutOfDateKHR;
    }

    if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR || framebufferResized) {
        framebufferResized = false;
        updateSwapchainStack();
    } else if (presentResult != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to acquire swapchain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void RTXApplication::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<RTXApplication*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;

    glfwGetWindowSize(window, &app->windowWidth, &app->windowHeight);
}

RTXApplication::~RTXApplication() {
    glfwDestroyWindow(window);
    glfwTerminate();
}