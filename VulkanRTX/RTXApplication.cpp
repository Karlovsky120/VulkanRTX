#include "RTXApplication.h"

#include "Buffer.h"
#include "MemoryAllocator.h"
#include "RayTracing.h"
#include "Vertex.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
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
    CommandPools::init(commandPools);

    swapchain = std::make_unique<Swapchain>(
        vkCtx->m_physicalDevice,
        *vkCtx->m_surface,
        *vkCtx->m_logicalDevice,
        vkCtx->m_presentQueue);

    //objectData = ObjLoader::loadObj(modelPath);

    /*chunk = std::make_unique<Chunk>();
    chunk->generateRandomly();
    chunk->generateVertices();
    //chunk.generateGreedyTriangles();
    chunk->generateGreedyTrianglesMultithreaded();
    //chunk.generateSimpleTriangles();*/

    chunk = std::make_unique<Chunk>();
    std::vector<Vertex> vertices = chunk->generateVertices();
    std::vector<uint32_t> indices = chunk->generateChunk(0);

    vertexBuffer = std::make_unique<Buffer>(
        *vkCtx->m_logicalDevice,
        vertices.size() * sizeof(Vertex),
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        vk::MemoryAllocateFlagBits::eDeviceAddress
        );

    object = std::make_unique<Mesh>(*vkCtx->m_logicalDevice, /*vertices, sizeof(Vertex),*/ indices);
    object->translate(glm::vec3(0.0, -CHUNK_SIZE * 0.5, 0));

    swapchainFrameInfos.resize(swapchain->m_imageCount);

    for (uint32_t i = 0; i < swapchainFrameInfos.size(); ++i) {
        swapchainFrameInfos[i].frameBuffer = std::make_unique<CommandBuffer>(PoolType::eCompute);
        //generateSwapchainFrameInfo(i);
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
    
    uniformBuffer = std::make_unique<Buffer>(
        *vkCtx->m_logicalDevice,
        sizeof(UniformBufferData),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    rt = std::make_unique<RayTracing>();
    descriptorPool = rt->createDescriptorPool();
    descriptorSetLayout = rt->createDescriptorSetLayout();
    descriptorSet = rt->createDescriptorSet(
        *descriptorPool,
        *descriptorSetLayout
    );

    AccelerationStructures asses;
    blas = std::make_unique<AccelerationStructure>(asses.createBottomAccelerationStructure(*object, vertexBuffer->get()));
    tlas = std::make_unique<AccelerationStructure>(asses.createTopAccelerationStructure(*blas));

    storageImage = rt->createStorageImage(windowWidth, windowHeight);

    rtPipeline = std::make_unique<RTPipeline>(*vkCtx->m_logicalDevice);
    rtPipeline->createPipeline(*descriptorSetLayout);

    sbt = rt->createSBTable(rtPipeline->get());

    updateDescriptorSets(
        *descriptorSet,
        *tlas->structure,
        storageImage->getView());

    /*for (int i = 0; i < swapchain->m_imageCount; ++i) {
        recordCommandBuffer(i);
    }*/

    glfwGetCursorPos(window, &cursorX, &cursorY);
}

void RTXApplication::initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(windowWidth, windowHeight, "Vulkan shenanigans", nullptr, nullptr);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
}

void RTXApplication::initOther() {
    camera = Camera(glm::vec3(0.0f, 0.0f, 2.5f), glm::vec3(0.0f), 90.0f, windowWidth / (float)windowHeight, 0.01f, 10000.0f);
}

void RTXApplication::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        calculateTime();
        updateFPS();
        processMouse();
        processKeyboard();

        uint32_t swapchainImageIndex;

        vk::FenceCreateInfo fenceInfo;
        vk::UniqueFence fence = vkCtx->m_logicalDevice->createFenceUnique(fenceInfo);

        vkCtx->m_logicalDevice->acquireNextImageKHR(
            *swapchain->m_swapchain,
            UINT64_MAX,
            nullptr,
            *fence,
            &swapchainImageIndex);

        vkCtx->m_logicalDevice->waitForFences(*fence, VK_TRUE, UINT64_MAX);

        recordCommandBuffer(swapchainImageIndex);

        swapchainFrameInfos[swapchainImageIndex].frameBuffer->submit(true);

        vk::PresentInfoKHR presentInfo;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &*swapchain->m_swapchain;
        presentInfo.pImageIndices = &swapchainImageIndex;

        vk::Result presentResult = vkCtx->m_presentQueue.presentKHR(presentInfo);
    }
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

    camera.rotate(-0.1f * glm::radians(deltaY), -0.1f * glm::radians(deltaX));

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
        translateVector.y += 1.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        translateVector.y -= 1.0f;
    }

    if (glm::length(translateVector) > 0.0000001f) {
        camera.translateOriented(glm::normalize(translateVector) * velocity * frameTime);
    }
}

void RTXApplication::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<RTXApplication*>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;

    glfwGetWindowSize(window, &app->windowWidth, &app->windowHeight);
}

RTXApplication::RTXApplication(std::string modelPath) :
    modelPath(modelPath) {}

RTXApplication::~RTXApplication() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void RTXApplication::updateDescriptorSets(
    vk::DescriptorSet& descriptorSet,
    vk::AccelerationStructureKHR& as,
    vk::ImageView& imageView) {

    vk::WriteDescriptorSetAccelerationStructureKHR descriptorSetAccelerationStructureInfo;
    descriptorSetAccelerationStructureInfo.accelerationStructureCount = 1;
    descriptorSetAccelerationStructureInfo.pAccelerationStructures = &as;

    vk::WriteDescriptorSet accelerationStructureWrite;
    accelerationStructureWrite.pNext = &descriptorSetAccelerationStructureInfo;
    accelerationStructureWrite.dstSet = descriptorSet;
    accelerationStructureWrite.dstBinding = 0;
    accelerationStructureWrite.descriptorCount = 1;
    accelerationStructureWrite.descriptorType = vk::DescriptorType::eAccelerationStructureKHR;

    vk::DescriptorImageInfo imageDescriptorInfo;
    imageDescriptorInfo.imageView = imageView;
    imageDescriptorInfo.imageLayout = vk::ImageLayout::eGeneral;

    vk::WriteDescriptorSet resultImageWrite;
    resultImageWrite.dstSet = descriptorSet;
    resultImageWrite.descriptorType = vk::DescriptorType::eStorageImage;
    resultImageWrite.dstBinding = 1;
    resultImageWrite.descriptorCount = 1;
    resultImageWrite.pImageInfo = &imageDescriptorInfo;

    vk::DescriptorBufferInfo descriptorBufferInfo;
    descriptorBufferInfo.buffer = uniformBuffer->get();
    descriptorBufferInfo.range = sizeof(UniformBufferData);
    descriptorBufferInfo.offset = 0;

    vk::WriteDescriptorSet uniformBufferWrite;
    uniformBufferWrite.dstSet = descriptorSet;
    uniformBufferWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
    uniformBufferWrite.dstBinding = 2;
    uniformBufferWrite.descriptorCount = 1;
    uniformBufferWrite.pBufferInfo = &descriptorBufferInfo;

    std::vector<vk::WriteDescriptorSet> writeDescriptorSets = {
        accelerationStructureWrite,
        resultImageWrite,
        uniformBufferWrite
    };

    vkCtx->m_logicalDevice->updateDescriptorSets(writeDescriptorSets, nullptr);
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
        //updateSwapchainStack();
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

void RTXApplication::generateSwapchainFrameInfo(const uint32_t index) {
    /*SwapchainFrameInfo& frameInfo = swapchainFrameInfos[index];

    frameInfo.descriptorBufferInfo.buffer = uniformBuffer->get();
    frameInfo.descriptorBufferInfo.offset = 0;
    frameInfo.descriptorBufferInfo.range = sizeof(UniformBufferObject);

    frameInfo.writeDescriptorSet.dstSet = *frameInfo.descriptorSet;
    frameInfo.writeDescriptorSet.dstBinding = 0;
    frameInfo.writeDescriptorSet.dstArrayElement = 0;
    frameInfo.writeDescriptorSet.descriptorType = vk::DescriptorType::eUniformBuffer;
    frameInfo.writeDescriptorSet.descriptorCount = 1;
    frameInfo.writeDescriptorSet.pBufferInfo = &frameInfo.descriptorBufferInfo;

    vkCtx->m_logicalDevice->updateDescriptorSets(1, &frameInfo.writeDescriptorSet, 0, nullptr);*/
}

/*void RTXApplication::updateSwapchainStack() {

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
}*/

/*void RTXApplication::updateSwapchainFrameInfo(const uint32_t index) {
    SwapchainFrameInfo& frameInfo = swapchainFrameInfos[index];

    frameInfo.renderPassBeginInfo.renderPass = *pipeline->m_renderPass;
    frameInfo.renderPassBeginInfo.framebuffer = *swapchain->m_framebuffers[index];
    frameInfo.renderPassBeginInfo.renderArea.extent = swapchain->m_extent;
}*/


void RTXApplication::recordCommandBuffer(const uint32_t index) {
    vk::StridedBufferRegionKHR raygenShaderSbtEntry;
    raygenShaderSbtEntry.buffer = sbt->get();
    raygenShaderSbtEntry.offset =
        static_cast<vk::DeviceSize>(vkCtx->m_rayTracingProperties.shaderGroupHandleSize * INDEX_RAYGEN);
    raygenShaderSbtEntry.size = vkCtx->m_rayTracingProperties.shaderGroupHandleSize;

    vk::StridedBufferRegionKHR missShaderSbtEntry;
    missShaderSbtEntry.buffer = sbt->get();
    missShaderSbtEntry.offset =
        static_cast<vk::DeviceSize>(vkCtx->m_rayTracingProperties.shaderGroupHandleSize * INDEX_MISS);
    missShaderSbtEntry.size = vkCtx->m_rayTracingProperties.shaderGroupHandleSize;

    vk::StridedBufferRegionKHR hitShaderSbtEntry;
    hitShaderSbtEntry.buffer = sbt->get();
    hitShaderSbtEntry.offset =
        static_cast<vk::DeviceSize>(vkCtx->m_rayTracingProperties.shaderGroupHandleSize * INDEX_CLOSEST_HIT);
    hitShaderSbtEntry.size = vkCtx->m_rayTracingProperties.shaderGroupHandleSize;

    vk::StridedBufferRegionKHR callableShaderSbtEntry;

    std::vector<vk::DescriptorSet> sets = { *descriptorSet };
    
    CommandBuffer& cmdBuffer = *swapchainFrameInfos[index].frameBuffer;
    cmdBuffer.reset();

    cmdBuffer.get().bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, rtPipeline->get());
    cmdBuffer.get().bindDescriptorSets(
        vk::PipelineBindPoint::eRayTracingKHR,
        rtPipeline->getLayout(),
        0,
        sets,
        nullptr);

    cmdBuffer.get().traceRaysKHR(
        &raygenShaderSbtEntry,
        &missShaderSbtEntry,
        &hitShaderSbtEntry,
        &callableShaderSbtEntry,
        windowWidth,
        windowHeight,
        1);

    vk::ImageSubresourceRange subresourceRange;
    subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;

    cmdBuffer.setImageLayout(
        storageImage->get(),
        vk::ImageLayout::eGeneral,
        vk::ImageLayout::eTransferSrcOptimal,
        subresourceRange);

    cmdBuffer.setImageLayout(
        swapchain->getImage(index),
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal,
        subresourceRange);

    vk::ImageSubresourceLayers subresourceLayers;
    subresourceLayers.aspectMask = vk::ImageAspectFlagBits::eColor;
    subresourceLayers.mipLevel = 0;
    subresourceLayers.baseArrayLayer = 0;
    subresourceLayers.layerCount = 1;

    vk::ImageCopy copyRegion;
    copyRegion.srcSubresource = subresourceLayers;
    copyRegion.srcOffset = vk::Offset3D(0, 0, 0);
    copyRegion.dstSubresource = subresourceLayers;
    copyRegion.dstOffset = vk::Offset3D(0, 0, 0);
    copyRegion.extent = vk::Extent3D(
        static_cast<uint32_t>(windowWidth),
        static_cast<uint32_t>(windowHeight),
        1);

    cmdBuffer.get().copyImage(
        storageImage->get(),
        vk::ImageLayout::eTransferSrcOptimal,
        swapchain->getImage(index),
        vk::ImageLayout::eTransferDstOptimal,
        copyRegion);

    cmdBuffer.setImageLayout(
        swapchain->getImage(index),
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        subresourceRange);

    cmdBuffer.setImageLayout(
        storageImage->get(),
        vk::ImageLayout::eTransferSrcOptimal,
        vk::ImageLayout::eGeneral,
        subresourceRange);
}

/*void RTXApplication::resetCommandBuffer(const uint32_t index) {
    SwapchainFrameInfo& frameInfo = swapchainFrameInfos[index];

    if (frameInfo.imageInUse != VK_NULL_HANDLE) {
        vkCtx->m_logicalDevice->waitForFences(
            frameInfo.imageInUse,
            VK_TRUE,
            UINT64_MAX);
    }
    frameInfo.frameBuffer.reset(vk::CommandBufferResetFlags());
}*/





/*void RTXApplication::updatePushConstants() {
    //object->rotate(glm::vec3(0.0f, -2 * frameTime, 0.0f));

    UniformBufferObject ubo;
    ubo.model = object->getMeshMatrix();
    ubo.playerPosition = camera.getCameraPosition();
    //lightPosition = glm::rotateY(lightPosition, 2 * frameTime);
    ubo.lightPosition = lightPosition;
    ubo.lightColor = lightColor;

    uniformBuffer->uploadToHostLocal(ubo);
}*/

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
    submitInfo.pCommandBuffers = &swapchainFrameInfo.frameBuffer->get();

    vk::Semaphore renderComplete[] = { *flightFrameInfo.renderCompleteSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = renderComplete;

    vkCtx->m_logicalDevice->resetFences(1, &*flightFrameInfo.inFlightFence);
    swapchainFrameInfo.frameBuffer->get().end();
    vkCtx->m_computeQueue.submit(submitInfo, *flightFrameInfo.inFlightFence);

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
        //updateSwapchainStack();
    } else if (presentResult != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to acquire swapchain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}