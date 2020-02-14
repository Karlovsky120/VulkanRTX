#pragma once

#include "Device.h"

#include <vulkan/vulkan.hpp>

class Instance {
public:
    vk::Instance& getInstance();

    Instance(const bool validationLayersEnabled);
    ~Instance();

    Instance(const Instance&) = delete;
    Instance& operator=(const Instance&) = delete;

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
