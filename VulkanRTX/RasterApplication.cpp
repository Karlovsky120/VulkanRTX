#include "RasterApplication.h"

#include "Buffer.h"
#include "CommandPools.h"
#include "MemoryAllocator.h"
#include "Vertex.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <chrono>
#include <iostream>

void RasterApplication::run() {
    initWindow();
    initVulkan();
    initOther();
    mainLoop();
}

void RasterApplication::initVulkan() {

    VulkanContext::init(vkCtx, window);
    MemoryAllocator::init(
        memoryAllocator,
        *vkCtx->m_logicalDevice,
        vkCtx->m_deviceMemoryProperties);
    CommandPools::init(commandPools);

    swapchain = std::make_unique<Swapchain>(
        vkCtx->m_physicalDevice,
        *vkCtx->m_surface,
        *vkCtx->m_logicalDevice,
        vkCtx->m_presentQueue);

    descriptorSetLayout = vkCtx->createUniformDescriptorSetLayout();

    pipeline = std::make_unique<Pipeline>(*vkCtx->m_logicalDevice);
    pipeline->initPipeline(swapchain->m_extent, swapchain->m_colorFormat, &*descriptorSetLayout);

    createDepthBuffer();

    clearColors.resize(2);
    clearColors[0].color = vk::ClearColorValue(std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f }));
    clearColors[1].depthStencil = vk::ClearDepthStencilValue(1.0f);

    swapchain->updateFramebuffers(*pipeline->m_renderPass, depthBuffer->getView());

    //objectData = ObjLoader::loadObj(modelPath);

    chunk = std::make_unique<Chunk>();
    std::vector<Vertex> vertices = chunk->generateVertices();

    vertexBuffer = std::make_unique<Buffer>(
        *vkCtx->m_logicalDevice,
        vertices.size() * sizeof(Vertex),
        vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    vertexBuffer->uploadToBuffer(vertices);

    std::vector<Vertex> dummy = {};

    uint32_t triangleCount = 0;

    for (uint32_t i = 0; i < 16; ++i) {
        for (uint32_t j = 0; j < 16; ++j) {
            std::vector<uint32_t> indices = chunk->generateChunk(16 * i + j);
            triangleCount += indices.size();
            chunks.push_back(std::make_unique<Mesh>(*vkCtx->m_logicalDevice, /*dummy, sizeof(Vertex),*/ indices));
            chunks.back()->translate(glm::vec3((i+1)*CHUNK_SIZE, 0.0f, (j+1)*CHUNK_SIZE));
        }
    }

    triangleCount /= 3;

    /*chunk->generateRandomly();
    chunk->generateVertices();
    //chunk.generateGreedyTriangles();
    chunk->generateGreedyTrianglesMultithreaded();
    //chunk.generateSimpleTriangles();


    object = std::make_unique<Mesh>(*vkCtx->m_logicalDevice, chunk->vertices, sizeof(Vertex), chunk->indices);
    object->translate(glm::vec3(0.0f, 0.0f, CHUNK_SIZE));*/
    
    uniformBuffer = std::make_unique<Buffer>(
        *vkCtx->m_logicalDevice,
        sizeof(UniformBufferObject),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    descriptorPool = vkCtx->createDescriptorPool(swapchain->m_imageCount);

    swapchainFrameInfos.resize(swapchain->m_imageCount);
    
    std::vector<vk::DescriptorSetLayout> layouts(swapchain->m_imageCount, *descriptorSetLayout);
    std::vector<vk::UniqueDescriptorSet> descriptorSets = vkCtx->createDescriptorSets(*descriptorPool, layouts);

    for (uint32_t i = 0; i < swapchainFrameInfos.size(); ++i) {
        swapchainFrameInfos[i].descriptorSet = std::move(descriptorSets[i]);

        swapchainFrameInfos[i].frameCommandBuffer = std::make_unique<CommandBuffer>(PoolType::eGraphics);

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

void RasterApplication::initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(windowWidth, windowHeight, "Vulkan shenanigans", nullptr, nullptr);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
}

void RasterApplication::initOther() {
    camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f), 90.0f, windowWidth / (float)windowHeight, 0.01f, 10000.0f);
}

void RasterApplication::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        calculateTime();
        updateFPS();
        processMouse();
        processKeyboard();

        uint32_t swapchainIndex = acquireNextImage();

        if (swapchainIndex != -1) {
            updatePushConstants();
            resetCommandBuffer(swapchainIndex);
            recordCommandBuffer(swapchainIndex);
            drawFrame(swapchainIndex);
        }
    }

    vkCtx->m_logicalDevice->waitIdle();
}

void RasterApplication::generateSwapchainFrameInfo(const uint32_t index) {
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

    frameInfo.renderPassBeginInfo.clearValueCount = clearColors.size();
    frameInfo.renderPassBeginInfo.pClearValues = clearColors.data();
    frameInfo.renderPassBeginInfo.renderArea.offset = vk::Offset2D(0, 0);
}

void RasterApplication::createDepthBuffer() {
    vk::SurfaceCapabilitiesKHR surfaceCapabilities =
        vkCtx->m_physicalDevice.getSurfaceCapabilitiesKHR(*vkCtx->m_surface);

    vk::Extent2D currentExtent;
    if (surfaceCapabilities.currentExtent.width != -1) {
        currentExtent = surfaceCapabilities.currentExtent;
    }

    std::vector<uint32_t> queueIndices = { vkCtx->m_graphicsQueueIndex };

    depthBuffer = std::make_unique<Image>(
        *vkCtx->m_logicalDevice,
        currentExtent.width,
        currentExtent.height,
        vk::Format::eD32Sfloat,
        vk::ImageUsageFlagBits::eDepthStencilAttachment,
        vk::ImageAspectFlagBits::eDepth);
}

void RasterApplication::updateSwapchainStack() {

    camera.updateProjectionMatrixAspectRatio(windowWidth / (float)windowHeight);

    for (uint32_t i = 0; i < swapchainFrameInfos.size(); ++i) {
        resetCommandBuffer(i);
    }

    // First update the swapchain according to the surface change
    swapchain->updateSwapchain();

    // Use new values of the swapchain to update the pipeline and renderpass
    pipeline->updatePipeline(swapchain->m_colorFormat, swapchain->m_extent);

    // Completely recreate the depth buffer;
    createDepthBuffer();

    // Use the newly created renderpass to update the framebuffers
    swapchain->updateFramebuffers(*pipeline->m_renderPass, depthBuffer->getView());

    // Update command buffers to use the updated objects
    for (uint32_t i = 0; i < swapchainFrameInfos.size(); ++i) {

        // Update info structures
        updateSwapchainFrameInfo(i);

        // Rerecord the command buffers
        recordCommandBuffer(i);
    }
}

void RasterApplication::updateSwapchainFrameInfo(const uint32_t index) {
    SwapchainFrameInfo& frameInfo = swapchainFrameInfos[index];

    frameInfo.renderPassBeginInfo.renderPass = *pipeline->m_renderPass;
    frameInfo.renderPassBeginInfo.framebuffer = *swapchain->m_framebuffers[index];
    frameInfo.renderPassBeginInfo.renderArea.extent = swapchain->m_extent;
}

void RasterApplication::recordCommandBuffer(const uint32_t index) {
    SwapchainFrameInfo& frameInfo = swapchainFrameInfos[index];
    
    vk::CommandBuffer& frameBuffer = frameInfo.frameCommandBuffer->get();

    frameBuffer.begin(frameInfo.beginInfo);
    frameBuffer.beginRenderPass(frameInfo.renderPassBeginInfo, vk::SubpassContents::eInline);
    frameBuffer.pushConstants(
        *pipeline->m_pipelineLayout,
        vk::ShaderStageFlagBits::eVertex,
        0,
        sizeof(glm::mat4),
        glm::value_ptr(camera.getCameraMatrix()));

    frameBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline->m_pipeline);
    frameBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        *pipeline->m_pipelineLayout,
        0,
        *frameInfo.descriptorSet,
        { nullptr });

    frameBuffer.bindVertexBuffers(0, vertexBuffer->get(), vk::DeviceSize(0));

    for (uint32_t i = 0; i < chunks.size(); ++i) {
        frameBuffer.pushConstants(
            *pipeline->m_pipelineLayout,
            vk::ShaderStageFlagBits::eVertex,
            sizeof(glm::mat4),
            sizeof(uint32_t),
            &i);

        frameBuffer.bindIndexBuffer(chunks[i]->getIndexBuffer(), 0, vk::IndexType::eUint32);
        frameBuffer.drawIndexed(chunks[i]->getIndexCount(), 1, 0, 0, 0);
    }

    frameBuffer.endRenderPass();
    frameBuffer.end();
}

void RasterApplication::resetCommandBuffer(const uint32_t index) {
    SwapchainFrameInfo& frameInfo = swapchainFrameInfos[index];

    if (frameInfo.imageInUse != vk::Fence()) {
        vkCtx->m_logicalDevice->waitForFences(
            frameInfo.imageInUse,
            VK_TRUE,
            UINT64_MAX);
    }
    frameInfo.frameCommandBuffer->get().reset(vk::CommandBufferResetFlags());
}

void RasterApplication::calculateTime() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    frameTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - time).count();

    time = currentTime;
}

void RasterApplication::updateFPS() {
    skip += frameTime;
    if (skip > 0.2f) {
        windowTitle = "Vulkan shenaningans FPS: " + std::to_string(1.0f / frameTime);
        skip = 0;
    }

    glfwSetWindowTitle(window, windowTitle.c_str());
}

void RasterApplication::processMouse() {
    double newCursorX;
    double newCursorY;
    glfwGetCursorPos(window, &newCursorX, &newCursorY);

    double deltaX = newCursorX - cursorX;
    double deltaY = newCursorY - cursorY;

    camera.rotate(-0.1f * glm::radians(deltaY), -0.1f * glm::radians(deltaX));

    cursorX = newCursorX;
    cursorY = newCursorY;
}

void RasterApplication::processKeyboard() {
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
        translateVector.y += 1.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        translateVector.y -= 1.0f;
    }

    if (glm::length(translateVector) > 0.0000001f) {
        camera.translateOriented(glm::normalize(translateVector) * velocity * frameTime);
    }
}

uint32_t RasterApplication::acquireNextImage() {
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

    if (swapchainInfo.imageInUse != vk::Fence()) {
        vkCtx->m_logicalDevice->waitForFences(swapchainInfo.imageInUse, VK_TRUE, UINT64_MAX);
    }

    swapchainInfo.imageInUse = *frameInfo.inFlightFence;

    return swapchainIndex;
}

void RasterApplication::updatePushConstants() {
    //object->rotate(glm::vec3(0.0f, -2 * frameTime, 0.0f));

    UniformBufferObject ubo;
    for (uint32_t i = 0; i < chunks.size(); ++i) {
        ubo.models[i] = chunks[i]->getMeshMatrix();
    }

    //ubo.model = chunks[0]->getMeshMatrix();
    ubo.playerPosition = camera.getCameraPosition();
    //lightPosition = glm::rotateY(lightPosition, 2 * frameTime);
    ubo.lightPosition = lightPosition;
    ubo.lightColor = lightColor;

    uniformBuffer->uploadToBuffer(ubo);
}

void RasterApplication::drawFrame(const uint32_t swapchainIndex) {
    SwapchainFrameInfo& swapchainFrameInfo = swapchainFrameInfos[swapchainIndex];
    InFlightFrameInfo& flightFrameInfo = inFlightFrameInfos[currentFrame];

    vk::SubmitInfo submitInfo;
    submitInfo.waitSemaphoreCount = 1;

    vk::Semaphore imageAvailable[] = { *flightFrameInfo.imageAvailableSemaphore };
    submitInfo.pWaitSemaphores = imageAvailable;

    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;

    submitInfo.pCommandBuffers = &swapchainFrameInfo.frameCommandBuffer->get();

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

void RasterApplication::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<RasterApplication*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;

    glfwGetWindowSize(window, &app->windowWidth, &app->windowHeight);
}

RasterApplication::RasterApplication(std::string modelPath) :
    modelPath(modelPath) {}

RasterApplication::~RasterApplication() {
    glfwDestroyWindow(window);
    glfwTerminate();
}