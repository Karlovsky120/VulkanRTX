#pragma once

#include "VulkanDevice.h"

#include <vulkan/vulkan.hpp>

class VulkanInstance {
public:
    vk::Instance& getInstance();

    VulkanInstance(const bool validationLayersEnabled);
    ~VulkanInstance();

    VulkanInstance(const VulkanInstance&) = delete;
    VulkanInstance& operator=(const VulkanInstance&) = delete;

private:
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_LUNARG_api_dump",
        "VK_LAYER_KHRONOS_validation"
    };

    vk::Instance instance;
    vk::DispatchLoaderDynamic loader;
    vk::DebugUtilsMessengerEXT debugMessenger;

    const bool validationLayersEnabled;

    const std::vector<const char*> getRequiredExtensions() const;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    );
};
