#pragma once

#include "Camera.h"
#include "Mesh.h"
#include "Pipeline.h"
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

    ~RTXApplication();

private:
    struct UniformBufferObject {
        glm::mat4 model;
    };

    struct SwapchainFrameInfo {
        vk::UniqueDescriptorSet descriptorSet;
        vk::DescriptorBufferInfo descriptorBufferInfo;
        vk::WriteDescriptorSet writeDescriptorSet;
        vk::UniqueCommandBuffer frameBuffer;
        vk::CommandBufferBeginInfo beginInfo;
        vk::RenderPassBeginInfo renderPassBeginInfo;
        vk::ClearValue clearColor;
        vk::Fence imageInUse = vk::Fence();
    };

    std::vector<vk::UniqueDescriptorSet> swapchainDescriptorSets;
    std::vector<vk::UniqueCommandBuffer> swapchainCmdBuffers;

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

    VulkanContext vkCtx;
    std::unique_ptr<Swapchain> swapchain;
    std::unique_ptr<Pipeline> pipeline;

    std::unique_ptr<Buffer> uniformBuffer;

    vk::UniqueDescriptorSetLayout descriptorSetLayout;
    vk::UniqueDescriptorPool descriptorPool;

    vk::UniqueCommandBuffer transferCmdBuffer;

    std::vector<SwapchainFrameInfo> swapchainFrameInfos;
    std::vector<InFlightFrameInfo> inFlightFrameInfos;

    std::unique_ptr<Mesh> object;

    vk::UniqueSemaphore flushStagingSemaphore;

    double cursorX;
    double cursorY;

    uint32_t currentFrame = 0;
    bool framebufferResized = false;

    void initVulkan();
    void initWindow();
    void initOther();
    void mainLoop();

    void flushStagingBuffers();
    void generateSwapchainFrameInfo(const uint32_t index);

    void updateSwapchainStack();
    void updateSwapchainFrameInfo(const uint32_t index);
    void recordCommandBuffer(const uint32_t index);

    void resetCommandBuffer(const uint32_t index);

    void calculateTime();
    void updateFPS();
    void processMouse();
    void processKeyboard();
    uint32_t acquireNextImage();
    void updateUniformBuffer();
    void drawFrame(const uint32_t swapchainIndex);

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
};
