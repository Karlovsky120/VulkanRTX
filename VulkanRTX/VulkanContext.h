#pragma once

#include <glm/mat4x4.hpp>
#include "VulkanInclude.h"

#include <vector>

struct GLFWwindow;

struct UniformBufferObject {
    glm::mat4 model;
};

class VulkanContext {
public:
    void createInstance();
    void createSurface(GLFWwindow* window);
    void createPhysicalDevice();
    void createLogicalDevice();

    void initContext(GLFWwindow* window);

    vk::UniqueDescriptorPool createDescriptorPool(const uint32_t descriptorSetCount) const;
    vk::UniqueDescriptorSetLayout createUniformDescriptorSetLayout() const;
    std::vector<vk::UniqueDescriptorSet> createDescriptorSets(
        const vk::DescriptorPool& descriptorPool,
        const std::vector<vk::DescriptorSetLayout>& setLayouts) const;

    vk::UniqueSemaphore createTimelineSemaphore(const uint32_t initialValue = 0) const;

    ~VulkanContext();

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
    uint32_t m_presentQueueIndex;

    vk::Queue m_graphicsQueue;
    vk::Queue m_transferQueue;
    vk::Queue m_presentQueue;

    bool m_rayTracingSupported = false;

#ifdef ENABLE_VALIDATION
private:
    vk::DebugUtilsMessengerEXT m_debugMessenger;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    );
#endif
};

