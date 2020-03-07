#pragma once

#include "Buffer.h"
#include "Camera.h"
#include "CommandBuffer.h"
#include "CommandPool.h"
#include "DescriptorPool.h"
#include "DescriptorSets.h"
#include "DescriptorSetLayout.h"
#include "DeviceMemory.h"
#include "Framebuffers.h"
#include "Instance.h"
#include "LogicalDevice.h"
#include "Mesh.h"
#include "PhysicalDevice.h"
#include "Pipeline.h"
#include "PipelineLayout.h"
#include "RenderPass.h"
#include "Surface.h"
#include "Swapchain.h"

#include <vulkan/vulkan.hpp>

#include <chrono>
#include <memory>
#include <string>

struct GLFWwindow;

class RTXApplication {
public:
    void run();

    ~RTXApplication();

private:
    const uint32_t WIDTH = 720; // 1280;
    const uint32_t HEIGHT = 720;

    const uint32_t MAX_FRAMES_IN_FLIGHT = 2;
    std::string windowTitle = "Vulkan shenanigans";

    Camera camera;

    std::chrono::time_point<std::chrono::high_resolution_clock> time;
    uint32_t skip;
    float frameTime;

    GLFWwindow* window;

    std::unique_ptr<Instance> instance;
    std::unique_ptr<Surface> surface;
    std::unique_ptr<PhysicalDevice> physicalDevice;
    std::unique_ptr<LogicalDevice> logicalDevice;
    std::unique_ptr<Swapchain> swapchain;
    std::unique_ptr<DescriptorSetLayout> descriptorSetLayout;
    std::unique_ptr<DescriptorPool> descriptorPool;
    std::unique_ptr<DescriptorSets> descriptorSets;
    std::unique_ptr<PipelineLayout> pipelineLayout;
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<Framebuffers> framebuffers;
    std::unique_ptr<RenderPass> renderPass;
    std::unique_ptr<CommandPool> graphicsCommandPool;
    std::unique_ptr<CommandPool> transferCommandPool;

    std::unique_ptr<CommandBuffer> transferBuffer;

    std::vector<std::unique_ptr<Buffer>> uniformBuffers;
    std::vector<std::unique_ptr<CommandBuffer>> commandBuffers;

    std::vector<vk::UniqueSemaphore> imageAvailableSemaphores;
    std::vector<vk::UniqueSemaphore> renderFinishedSemaphores;
    std::vector<vk::UniqueFence> inFlightFences;
    std::vector<vk::Fence> imagesInFlight;

    std::unique_ptr<Mesh> object;

    bool disableInput = false;

    double cursorX;
    double cursorY;

    size_t currentFrame = 0;
    bool framebufferResized = false;

    void initVulkan();
    void initWindow();
    void initOther();
    void mainLoop();
    void drawFrame();

    void processMouse();
    void processKeyboard();

    void updateUniformBuffer(uint32_t bufferIndex);
    void updatePushConstants(vk::CommandBuffer& cmdBuffer);

    void createSwapchainHierarchy();
    void deleteSwapchainHierarchy();

    void recreateSwapchainHierarchy();

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

    template <class T>
    void stageDataUploadToGPU(std::unique_ptr<Buffer>& hostBuffer,
                              std::unique_ptr<Buffer>& deviceBuffer,
                              vk::BufferCopy& bufferCopy,
                              vk::BufferUsageFlags usageFlags,
                              const std::vector<T>& data) {

        uint32_t sizeInBytes = data.size() * sizeof(T);

        hostBuffer = logicalDevice->createBuffer(sizeInBytes,
                                                 usageFlags | vk::BufferUsageFlagBits::eTransferSrc,
                                                 vk::MemoryPropertyFlagBits::eHostVisible);
        hostBuffer->copyToBuffer(data);
        deviceBuffer = logicalDevice->createBuffer(sizeInBytes,
                                                   usageFlags | vk::BufferUsageFlagBits::eTransferDst,
                                                   vk::MemoryPropertyFlagBits::eDeviceLocal);

        bufferCopy.srcOffset = 0;
        bufferCopy.srcOffset = 0;
        bufferCopy.size = sizeInBytes;
    }
};
