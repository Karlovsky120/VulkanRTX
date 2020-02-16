#pragma once

#include "CommandBuffer.h"
#include "CommandPool.h"
#include "Framebuffers.h"
#include "Instance.h"
#include "LogicalDevice.h"
#include "PhysicalDevice.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "Surface.h"
#include "Swapchain.h"

#include <vulkan/vulkan.hpp>

#include <memory>

struct GLFWwindow;

class RTXApplication {
public:
    void run();

    ~RTXApplication();

private:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    const uint32_t WIDTH = 1280;
    const uint32_t HEIGHT = 720;

    const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    GLFWwindow* window;

    std::unique_ptr<Instance> instance;
    std::unique_ptr<Surface> surface;
    std::unique_ptr<PhysicalDevice> physicalDevice;
    std::unique_ptr<LogicalDevice> logicalDevice;
    std::unique_ptr<Swapchain> swapchain;
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<Framebuffers> framebuffers;
    std::unique_ptr<RenderPass> renderPass;
    std::unique_ptr<CommandPool> commandPool;

    std::vector<std::unique_ptr<CommandBuffer>> commandBuffers;

    std::vector<vk::UniqueSemaphore> imageAvailableSemaphores;
    std::vector<vk::UniqueSemaphore> renderFinishedSemaphores;
    std::vector<vk::UniqueFence> inFlightFences;
    std::vector<vk::UniqueFence*> imagesInFlight;

    size_t currentFrame = 0;

    void initVulkan();
    void initWindow();
    void mainLoop();
    void drawFrame();
};
