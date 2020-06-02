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
        vkCtx->m_deviceMemoryProperties);
    CommandPools::init(commandPools);

    swapchain = std::make_unique<Swapchain>(
        vkCtx->m_physicalDevice,
        *vkCtx->m_surface,
        vkCtx->m_presentQueue);

    chunkGenerator = std::make_unique<ChunkGenerator>();
    std::vector<Vertex> vertices = chunkGenerator->generateVertices();

    uint32_t triangles = 0;

    for (uint32_t i = 0; i < CHUNK_DIM; ++i) {
        for (uint32_t j = 0; j < CHUNK_DIM; ++j) {
            std::vector<uint32_t> indices = chunkGenerator->generateChunk(CHUNK_DIM * i + j);
            chunks.push_back(std::make_unique<Mesh>(
                indices,
                "Index buffer for chunk " + std::to_string(i) + "x" + std::to_string(j)));
            chunks.back()->translate(glm::vec3((i+1) * CHUNK_SIZE, 0.0f, (j+1) * CHUNK_SIZE));
            triangles += indices.size();
        }
    }
    
    triangles /= 3;
    
    vertexBuffer = std::make_unique<Buffer>(
        vertices.size() * sizeof(Vertex),
        vk::BufferUsageFlagBits::eVertexBuffer
        | vk::BufferUsageFlagBits::eStorageBuffer
        | vk::BufferUsageFlagBits::eTransferDst
        | vk::BufferUsageFlagBits::eShaderDeviceAddress,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        "All chunk vertices",
        vk::MemoryAllocateFlagBits::eDeviceAddress);
    vertexBuffer->uploadToBuffer(vertices);
    
    swapchainFrameInfos.resize(swapchain->m_imageCount);

    for (uint32_t i = 0; i < swapchainFrameInfos.size(); ++i) {
        swapchainFrameInfos[i].frameBuffer = std::make_unique<CommandBuffer>(PoolType::eGraphics);
    }
    
    inFlightFrameInfos.resize(MAX_FRAMES_IN_FLIGHT);
    vk::SemaphoreCreateInfo semaphoreCreateInfo;
    vk::FenceCreateInfo fenceCreateInfo;
    fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
    
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        inFlightFrameInfos[i].imageAvailableSemaphore = VulkanContext::getDevice().createSemaphoreUnique(semaphoreCreateInfo);
        inFlightFrameInfos[i].renderCompleteSemaphore = VulkanContext::getDevice().createSemaphoreUnique(semaphoreCreateInfo);
        inFlightFrameInfos[i].inFlightFence = VulkanContext::getDevice().createFenceUnique(fenceCreateInfo);
    }
    
    uniformBuffer = std::make_unique<Buffer>(
        sizeof(UniformBufferObject),
        vk::BufferUsageFlagBits::eUniformBuffer
        | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal,
        "RTX uniform buffer");

    AccelerationStructures asses;
    std::vector<std::unique_ptr<AccelerationStructure>> blases;
    for (uint32_t i = 0; i < CHUNK_DIM; ++i) {
        for (uint32_t j = 0; j < CHUNK_DIM; ++j) {
            blases.push_back(
                std::make_unique<AccelerationStructure>(
                    asses.createBottomAccelerationStructure(
                        *chunks[i * CHUNK_DIM + j], vertexBuffer->get()
                    )
                )
            );
            chunkTransformations.push_back(chunks.back()->getMeshMatrix());
        }
    }

    tlas = std::make_unique<AccelerationStructure>(asses.createTopAccelerationStructure(blases));

    rt = std::make_unique<RayTracing>();
    descriptorPool = rt->createDescriptorPool();
    descriptorSetLayout = rt->createDescriptorSetLayout(chunks.size());
    descriptorSet = rt->createDescriptorSet(
        *descriptorPool,
        *descriptorSetLayout
    );

    rayTraceTarget = std::make_unique<Image>(
        windowWidth,
        windowHeight,
#ifdef OPTIX_DENOISER
        vk::Format::eR32G32B32A32Sfloat,
#else
        vk::Format::eB8G8R8A8Unorm,
#endif
        "RTX render target",
        vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferDst,
        vk::ImageAspectFlagBits::eColor,
        vk::ImageLayout::eGeneral);

#ifdef OPTIX_DENOISER
    denoiser = std::make_unique<Denoiser>();
#endif

    rtPipeline = std::make_unique<RTPipeline>();
    rtPipeline->createPipeline(*descriptorSetLayout);

    sbt = rt->createSBTable(rtPipeline->get());

    updateDescriptorSets(
        *descriptorSet,
        *tlas->structure,
        rayTraceTarget->getView(),
        chunks,
        vertexBuffer->get());

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
    camera = Camera(glm::vec3(0.0f, -32.0f, 2.5f), glm::vec3(0.0f), 90.0f, windowWidth / (float)windowHeight, 0.01f, 10000.0f);
}

void RTXApplication::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        calculateTime();
        updateFPS();
        processMouse();
        processKeyboard();

        updateUniformBuffer();

        uint32_t swapchainImageIndex;

        vk::FenceCreateInfo fenceInfo;
        vk::UniqueFence fence = VulkanContext::getDevice().createFenceUnique(fenceInfo);

        VulkanContext::getDevice().acquireNextImageKHR(
            *swapchain->m_swapchain,
            UINT64_MAX,
            nullptr,
            *fence,
            &swapchainImageIndex);

        VulkanContext::getDevice().waitForFences(*fence, VK_TRUE, UINT64_MAX);

        executeCommandBuffer(swapchainImageIndex);

        vk::PresentInfoKHR presentInfo;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &*swapchain->m_swapchain;
        presentInfo.pImageIndices = &swapchainImageIndex;

        vkCtx->m_presentQueue.presentKHR(presentInfo);
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

RTXApplication::~RTXApplication() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void RTXApplication::updateDescriptorSets(
    const vk::DescriptorSet& descriptorSet,
    const vk::AccelerationStructureKHR& as,
    const vk::ImageView& imageView,
    const std::vector<std::unique_ptr<Mesh>>& chunks,
    const vk::Buffer& vertexBuffer) {

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
    descriptorBufferInfo.offset = 0;
    descriptorBufferInfo.range = VK_WHOLE_SIZE;

    vk::WriteDescriptorSet uniformBufferWrite;
    uniformBufferWrite.dstSet = descriptorSet;
    uniformBufferWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
    uniformBufferWrite.dstBinding = 2;
    uniformBufferWrite.descriptorCount = 1;
    uniformBufferWrite.pBufferInfo = &descriptorBufferInfo;

    vk::DescriptorBufferInfo vertexBufferInfo;
    vertexBufferInfo.buffer = vertexBuffer;
    vertexBufferInfo.offset = 0;
    vertexBufferInfo.range = VK_WHOLE_SIZE;

    vk::WriteDescriptorSet vertexWrite;
    vertexWrite.dstSet = descriptorSet;
    vertexWrite.descriptorType = vk::DescriptorType::eStorageBuffer;
    vertexWrite.dstBinding = 3;
    vertexWrite.descriptorCount = 1;
    vertexWrite.pBufferInfo = &vertexBufferInfo;

    std::vector<vk::DescriptorBufferInfo> indices;
    for (uint32_t i = 0; i < chunks.size(); ++i) {
        vk::DescriptorBufferInfo info;
        info.buffer = chunks[i]->getIndexBuffer();
        info.offset = 0;
        info.range = VK_WHOLE_SIZE;
        
        indices.push_back(info);
    }

    vk::WriteDescriptorSet indexArrayWrite;
    indexArrayWrite.dstSet = descriptorSet;
    indexArrayWrite.descriptorType = vk::DescriptorType::eStorageBuffer;
    indexArrayWrite.dstBinding = 4;
    indexArrayWrite.descriptorCount = indices.size();
    indexArrayWrite.pBufferInfo = indices.data();

    std::vector<vk::WriteDescriptorSet> writeDescriptorSets = {
        accelerationStructureWrite,
        resultImageWrite,
        uniformBufferWrite,
        vertexWrite,
        indexArrayWrite
    };

    VulkanContext::getDevice().updateDescriptorSets(writeDescriptorSets, nullptr);
}

void RTXApplication::executeCommandBuffer(const uint32_t index) {
    vk::DeviceSize groupHandleSize = vkCtx->m_rayTracingProperties.shaderGroupHandleSize;

    vk::StridedBufferRegionKHR raygenShaderSbtEntry;
    raygenShaderSbtEntry.buffer = sbt->get();
    raygenShaderSbtEntry.offset =
        static_cast<vk::DeviceSize>(groupHandleSize * INDEX_RAYGEN);
    raygenShaderSbtEntry.size = groupHandleSize;

    vk::StridedBufferRegionKHR missShaderSbtEntry;
    missShaderSbtEntry.buffer = sbt->get();
    missShaderSbtEntry.offset =
        static_cast<vk::DeviceSize>(groupHandleSize * INDEX_MISS);
    missShaderSbtEntry.size = groupHandleSize;

    vk::StridedBufferRegionKHR shadowMissShaderSbtEntry;
    shadowMissShaderSbtEntry.buffer = sbt->get();
    shadowMissShaderSbtEntry.offset =
        static_cast<vk::DeviceSize>(groupHandleSize * INDEX_SHADOW_MISS);
    shadowMissShaderSbtEntry.size = groupHandleSize;
    
    vk::StridedBufferRegionKHR hitShaderSbtEntry;
    hitShaderSbtEntry.buffer = sbt->get();
    hitShaderSbtEntry.offset =
        static_cast<vk::DeviceSize>(groupHandleSize * INDEX_CLOSEST_HIT);
    hitShaderSbtEntry.size = groupHandleSize;

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

    glm::mat4 viewInv = glm::inverse(camera.getViewMatrix());
    cmdBuffer.get().pushConstants(
        rtPipeline->getLayout(),
        vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR,
        0,
        sizeof(glm::mat4),
        &viewInv);

    glm::mat4 projInv = glm::inverse(camera.getProjectionMatrix());
    cmdBuffer.get().pushConstants(
        rtPipeline->getLayout(),
        vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR,
        sizeof(glm::mat4),
        sizeof(glm::mat4),
        &projInv);

    glm::vec3 position = camera.getCameraPosition();
    cmdBuffer.get().pushConstants(
        rtPipeline->getLayout(),
        vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR,
        2 * sizeof(glm::mat4),
        sizeof(glm::vec3),
        &position);

    cmdBuffer.get().pushConstants(
        rtPipeline->getLayout(),
        vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR,
        2 * sizeof(glm::mat4) + sizeof(glm::vec3),
        sizeof(glm::vec3),
        &lightPosition);

    cmdBuffer.get().traceRaysKHR(
        &raygenShaderSbtEntry,
        &missShaderSbtEntry,
        &hitShaderSbtEntry,
        &callableShaderSbtEntry,
        windowWidth,
        windowHeight,
        1);

    cmdBuffer.setImageLayout(
        swapchain->getImage(index),
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal);

    vk::ImageSubresourceLayers subresourceLayers;
    subresourceLayers.aspectMask = vk::ImageAspectFlagBits::eColor;
    subresourceLayers.mipLevel = 0;
    subresourceLayers.baseArrayLayer = 0;
    subresourceLayers.layerCount = 1;

#ifdef OPTIX_DENOISER
    denoiser->denoise(
        cmdBuffer,
        *rayTraceTarget,
        vk::ImageLayout::eGeneral,
        *rayTraceTarget,
        vk::ImageLayout::eTransferSrcOptimal);

    vk::ImageBlit blit;
    blit.srcSubresource = subresourceLayers;
    blit.srcOffsets[0] = vk::Offset3D( 0, 0, 0 );
    blit.srcOffsets[1] = vk::Offset3D(windowWidth, windowHeight, 1 );
    blit.dstSubresource = subresourceLayers;
    blit.dstOffsets[0] = vk::Offset3D(0, 0, 0 );
    blit.dstOffsets[1] = vk::Offset3D(windowWidth, windowHeight, 1 );

    cmdBuffer.get().blitImage(
        rayTraceTarget->get(),
        vk::ImageLayout::eTransferSrcOptimal,
        swapchain->getImage(index),
        vk::ImageLayout::eTransferDstOptimal,
        blit,
        vk::Filter::eNearest);

#else
    cmdBuffer.setImageLayout(
        rayTraceTarget->get(),
        vk::ImageLayout::eGeneral,
        vk::ImageLayout::eTransferSrcOptimal);

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
        rayTraceTarget->get(),
        vk::ImageLayout::eTransferSrcOptimal,
        swapchain->getImage(index),
        vk::ImageLayout::eTransferDstOptimal,
        copyRegion);
#endif

    cmdBuffer.setImageLayout(
        rayTraceTarget->get(),
        vk::ImageLayout::eTransferSrcOptimal,
        vk::ImageLayout::eGeneral);

    cmdBuffer.setImageLayout(
        swapchain->getImage(index),
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::ePresentSrcKHR);

    cmdBuffer.submitAndWait();
}

void RTXApplication::updateUniformBuffer() { 
    ubo.projInv = glm::inverse(camera.getProjectionMatrix());
    ubo.viewInv = glm::inverse(camera.getViewMatrix());
    ubo.lightPosition = lightPosition;

    for (uint32_t i = 0; i < lightSpan.size(); ++i) {
        ubo.lightSpan[i] = lightSpan[i];
    }

    uniformBuffer->uploadToBuffer(ubo);
}