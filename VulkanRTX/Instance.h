#pragma once

#include <vulkan/vulkan.hpp>

class PhysicalDevice;
class Surface;

struct GLFWwindow;

class Instance {
public:
    vk::Instance& get();

    std::unique_ptr<PhysicalDevice> createPhysicalDevice();
    std::unique_ptr<Surface> createSurface(GLFWwindow* window, uint32_t width, uint32_t height);

    Instance();
    ~Instance();

    Instance(const Instance&) = delete;
    Instance& operator=(const Instance&) = delete;

private:
    vk::Instance m_instance;

    vk::DispatchLoaderDynamic m_loader;

#ifdef ENABLE_VALIDATION
    vk::DebugUtilsMessengerEXT m_debugMessenger;

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    );
#endif
};
