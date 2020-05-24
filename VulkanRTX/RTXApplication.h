#pragma once

#include "AccelerationStructure.h"
#include "Camera.h"
#include "Chunk.h"
#include "Image.h"
#include "Mesh.h"
#include "ObjLoader.h"
#include "Pipeline.h"
#include "RayTracing.h"
#include "RTPipeline.h"
#include "Swapchain.h"
#include "VulkanContext.h"

#include "VulkanInclude.h"
#include <glm/mat4x4.hpp>

#include <chrono>
#include <memory>
#include <string>

struct GLFWwindow;

class RTXApplication {
public:
    void run();

    RTXApplication(std::string modelPath);
    ~RTXApplication();

private:
    struct UniformBufferData {
        glm::mat4 viewInv;
        glm::mat4 projInv;
    };

    struct SwapchainFrameInfo {
        //vk::UniqueDescriptorSet descriptorSet;
        //vk::DescriptorBufferInfo descriptorBufferInfo;
        //vk::WriteDescriptorSet writeDescriptorSet;
        std::unique_ptr<CommandBuffer> frameBuffer;
        //vk::CommandBufferBeginInfo beginInfo;
        vk::Fence imageInUse = vk::Fence();
    };

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
    float skip;
    float frameTime;
    const float velocity = 4.0f; 

    GLFWwindow* window;

    std::string modelPath;
    //ObjectData objectData;
    std::unique_ptr<Chunk> chunk;
    std::unique_ptr<Buffer> vertexBuffer;

    std::shared_ptr<VulkanContext> vkCtx;
    std::shared_ptr<MemoryAllocator> memoryAllocator;
    std::shared_ptr<CommandPools> commandPools;

    std::shared_ptr<RayTracing> rt;
    
    std::unique_ptr<Swapchain> swapchain;

    std::unique_ptr<AccelerationStructure> blas;
    std::unique_ptr<AccelerationStructure> tlas;

    std::unique_ptr<RTPipeline> rtPipeline;
    std::unique_ptr<Buffer> sbt;

    vk::UniqueDescriptorPool descriptorPool;
    vk::UniqueDescriptorSetLayout descriptorSetLayout;
    vk::UniqueDescriptorSet descriptorSet;

    std::unique_ptr<Buffer> uniformBuffer;
    std::unique_ptr<Image> storageImage;

    /*std::unique_ptr<Pipeline> pipeline;
    
    std::unique_ptr<Buffer> uniformBuffer;
    
    vk::UniqueDescriptorSetLayout descriptorSetLayout;
    vk::UniqueDescriptorPool descriptorPool;*/
    
    std::vector<SwapchainFrameInfo> swapchainFrameInfos;
    std::vector<InFlightFrameInfo> inFlightFrameInfos;
    
    std::unique_ptr<Mesh> object;
    glm::vec3 lightPosition = glm::vec3(-20.0f, 20.0f, 0.0f);
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
    /*void createDepthBuffer();

    void updateSwapchainStack();
    void updateSwapchainFrameInfo(const uint32_t index);*/
    void recordCommandBuffer(const uint32_t index);

    /*void resetCommandBuffer(const uint32_t index);*/

    void calculateTime();
    void updateFPS();
    void processMouse();
    void processKeyboard();

    void updateDescriptorSets(
        vk::DescriptorSet& descriptorSet,
        vk::AccelerationStructureKHR& as,
        vk::ImageView& imageView);

    uint32_t acquireNextImage();
    /*void updatePushConstants();*/
    void drawFrame(const uint32_t swapchainIndex);

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};