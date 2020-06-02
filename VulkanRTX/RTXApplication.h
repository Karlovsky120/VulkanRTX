#pragma once

#include "GlobalDefines.h"

#include "AccelerationStructure.h"
#include "Camera.h"
#include "ChunkGenerator.h"
#include "Denoiser.h"
#include "Image.h"
#include "Mesh.h"
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

#define CHUNK_DIM 1

class RTXApplication {
public:
    void run();

    ~RTXApplication();

private:
    struct UniformBufferObject {
        glm::mat4 viewInv;
        glm::mat4 projInv;
        glm::vec3 playerPosition;
        uint32_t padding1;
        glm::vec3 lightPosition;
        uint32_t padding2;
    };

    struct SwapchainFrameInfo {
        std::unique_ptr<CommandBuffer> frameBuffer;
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
    std::string windowTitle = "Vulkan RTX shenanigans";

    Camera camera;

    std::chrono::time_point<std::chrono::high_resolution_clock> time;
    float skip;
    float frameTime;
    const float velocity = 16.0f; 

    GLFWwindow* window;

    std::unique_ptr<ChunkGenerator> chunkGenerator;

    std::shared_ptr<VulkanContext> vkCtx;
    std::shared_ptr<MemoryAllocator> memoryAllocator;
    std::shared_ptr<CommandPools> commandPools;

#ifdef OPTIX_DENOISER
    std::unique_ptr<Denoiser> denoiser;
#endif

    std::unique_ptr<RayTracing> rt;
    
    std::unique_ptr<Swapchain> swapchain;

    std::unique_ptr<AccelerationStructure> blas;
    std::unique_ptr<AccelerationStructure> tlas;

    std::unique_ptr<RTPipeline> rtPipeline;
    std::unique_ptr<Buffer> sbt;

    vk::UniqueDescriptorPool descriptorPool;
    vk::UniqueDescriptorSetLayout descriptorSetLayout;
    vk::UniqueDescriptorSet descriptorSet;

    std::vector<std::unique_ptr<Mesh>> chunks;
    std::unique_ptr<Buffer> vertexBuffer;
    std::unique_ptr<Buffer> uniformBuffer;
    std::unique_ptr<Image> rayTraceTarget;
    std::vector<glm::mat4> chunkTransformations;
    
    std::vector<SwapchainFrameInfo> swapchainFrameInfos;
    std::vector<InFlightFrameInfo> inFlightFrameInfos;
    
    glm::vec3 lightPosition = glm::vec3(0.0f, -64.0f, 0.0f);
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    
    vk::UniqueSemaphore flushStagingSemaphore;

    UniformBufferObject ubo;
    
    double cursorX;
    double cursorY;
    
    uint32_t currentFrame = 0;
    bool framebufferResized = false;
    
    void initVulkan();
    void initWindow();
    void initOther();
    void mainLoop();

    void executeCommandBuffer(const uint32_t index);

    void calculateTime();
    void updateFPS();
    void processMouse();
    void processKeyboard();
    void updateUniformBuffer();

    void updateDescriptorSets(
        const vk::DescriptorSet& descriptorSet,
        const vk::AccelerationStructureKHR& as,
        const vk::ImageView& imageView,
        const std::vector<std::unique_ptr<Mesh>>& chunks,
        const vk::Buffer& vertexBuffer);

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};