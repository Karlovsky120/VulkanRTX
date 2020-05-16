#pragma once

#include "Camera.h"
#include "Chunk.h"
#include "CommandBuffer.h"
#include "Image.h"
#include "Mesh.h"
#include "ObjLoader.h"
#include "Pipeline.h"
#include "Swapchain.h"
#include "VulkanContext.h"

#include "VulkanInclude.h"
#include <glm/mat4x4.hpp>

#include <chrono>
#include <memory>
#include <string>

struct GLFWwindow;

class RasterApplication {
public:
    void run();

    RasterApplication(std::string modelPath);
    ~RasterApplication();

private:
    struct UniformBufferObject {
        glm::mat4 model;
        glm::vec3 playerPosition;
        float spacer1;
        glm::vec3 lightPosition;
        float spacer2;
        glm::vec3 lightColor;
        float spacer3;
    };

    struct SwapchainFrameInfo {
        vk::UniqueDescriptorSet descriptorSet;
        vk::DescriptorBufferInfo descriptorBufferInfo;
        vk::WriteDescriptorSet writeDescriptorSet;
        std::unique_ptr<CommandBuffer> frameCommandBuffer;
        vk::CommandBufferBeginInfo beginInfo;
        vk::RenderPassBeginInfo renderPassBeginInfo;
        vk::Fence imageInUse = vk::Fence();
        std::unique_ptr<Image> rasterImage;
    };

    std::vector<vk::ClearValue> clearColors;

    struct InFlightFrameInfo {
        vk::UniqueSemaphore imageAvailableSemaphore;
        vk::UniqueSemaphore renderCompleteSemaphore;
        vk::UniqueFence inFlightFence;
    };

    int windowWidth = 1280;
    int windowHeight = 720;

    const uint32_t MAX_FRAMES_IN_FLIGHT = 2;
    std::string windowTitle = "Vulkan shenanigans";

    Camera camera;

    std::chrono::time_point<std::chrono::high_resolution_clock> time;
    float skip = 0;
    float frameTime = 1.0f;
    const float velocity = 8.0f; 

    GLFWwindow* window;

    std::string modelPath;
    ObjectData objectData;
    std::unique_ptr<Chunk> chunk;

    std::shared_ptr<VulkanContext> vkCtx;
    std::shared_ptr<MemoryAllocator> memoryAllocator;
    std::shared_ptr<CommandPools> commandPools;
    
    std::unique_ptr<Swapchain> swapchain;
    std::unique_ptr<Pipeline> pipeline;
    
    std::unique_ptr<Buffer> uniformBuffer;
    std::unique_ptr<Image> depthBuffer;
    
    vk::UniqueDescriptorSetLayout descriptorSetLayout;
    vk::UniqueDescriptorPool descriptorPool;
    
    std::vector<SwapchainFrameInfo> swapchainFrameInfos;
    std::vector<InFlightFrameInfo> inFlightFrameInfos;
    
    std::unique_ptr<Mesh> object;
    glm::vec3 lightPosition = glm::vec3(-32.0f, 0.0f, 0.0f);
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    vk::UniqueSemaphore flushStagingSemaphore;
    
    double cursorX;
    double cursorY;
    
    uint32_t currentFrame = 0;
    bool framebufferResized = false;
    
    void initVulkan();
    void initWindow();
    void initOther();
    void mainLoop();

    void generateSwapchainFrameInfo(const uint32_t index);
    void createDepthBuffer();

    void updateSwapchainStack();
    void updateSwapchainFrameInfo(const uint32_t index);
    void recordCommandBuffer(const uint32_t index);

    void resetCommandBuffer(const uint32_t index);

    void calculateTime();
    void updateFPS();
    void processMouse();
    void processKeyboard();
    uint32_t acquireNextImage();
    void updatePushConstants();
    void drawFrame(const uint32_t swapchainIndex);

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};
