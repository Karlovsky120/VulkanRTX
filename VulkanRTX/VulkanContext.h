#pragma once

#include "GlobalDefines.h"

#include <glm/mat4x4.hpp>
#include "VulkanInclude.h"

#ifdef AFTERMATH
#include <Aftermath/NsightAftermathGpuCrashTracker.h>
#endif

#include <vector>

struct GLFWwindow;

class VulkanContext {
public:
    static void init(std::shared_ptr<VulkanContext>& storage, GLFWwindow* window);
    static std::shared_ptr<VulkanContext> get();

    void createInstance();
    void createSurface(GLFWwindow* window);
    void createPhysicalDevice();
    void createLogicalDevice();

    static vk::Device& getDevice();

    void initContext(GLFWwindow* window);

    vk::UniqueDescriptorPool createDescriptorPool(const uint32_t descriptorSetCount) const;
    vk::UniqueDescriptorSetLayout createUniformDescriptorSetLayout() const;
    std::vector<vk::UniqueDescriptorSet> createDescriptorSets(
        const vk::DescriptorPool& descriptorPool,
        const std::vector<vk::DescriptorSetLayout>& setLayouts) const;

    vk::UniqueSemaphore createTimelineSemaphore(const uint32_t initialValue = 0) const;

    vk::DynamicLoader m_loader;
	vk::UniqueInstance m_instance;
    vk::UniqueSurfaceKHR m_surface;
    vk::PhysicalDevice m_physicalDevice;
    vk::UniqueDevice m_logicalDevice;

    vk::PhysicalDeviceProperties m_deviceProperties;
    vk::PhysicalDeviceFeatures m_deviceFeatures;
    vk::PhysicalDeviceMemoryProperties m_deviceMemoryProperties;
    vk::PhysicalDeviceRayTracingPropertiesKHR m_rayTracingProperties;

    uint32_t m_graphicsQueueIndex;
    uint32_t m_transferQueueIndex;
    uint32_t m_computeQueueIndex;
    uint32_t m_presentQueueIndex;

    vk::Queue m_graphicsQueue;
    vk::Queue m_transferQueue;
    vk::Queue m_computeQueue;
    vk::Queue m_presentQueue;

    bool m_rayTracingSupported = false;

#ifdef ENABLE_DEBUG_MARKERS
    bool m_debugMarkersSupported = false;
    static void nameObject(void* object, vk::DebugReportObjectTypeEXT type, std::string name);
#endif

#ifdef AFTERMATH
    bool m_aftermathSupported = false;
    GpuCrashTracker m_gpuCrashTracker;
#endif

    VulkanContext(VulkanContext const&) = delete;
    void operator=(VulkanContext const&) = delete;

    VulkanContext(GLFWwindow* window);

#ifdef ENABLE_VALIDATION
    ~VulkanContext();
#endif

private:
    uint32_t findTransferQueue(std::vector<vk::QueueFamilyProperties> queueFamilyProperties);
    uint32_t findComputeQueue(std::vector<vk::QueueFamilyProperties> queueFamilyProperties);
    uint32_t findGraphicsQueue(std::vector<vk::QueueFamilyProperties> queueFamilyProperties);
    uint32_t findPresentQueue(uint32_t count, vk::SurfaceKHR surface, vk::PhysicalDevice physicalDevice);

#ifdef ENABLE_VALIDATION
    vk::DebugUtilsMessengerEXT m_debugMessenger;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    );
#endif

    static std::weak_ptr<VulkanContext> m_staticInstance;
};

