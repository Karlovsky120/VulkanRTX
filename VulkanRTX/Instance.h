#pragma once

#include <vulkan/vulkan.hpp>

class PhysicalDevice;
class Surface;

struct GLFWwindow;

class Instance {
public:
    vk::Instance& get();

    std::unique_ptr<PhysicalDevice> createPhysicalDevice();
    std::unique_ptr<Surface> createSurface(GLFWwindow* window,
                                           const uint32_t width,
                                           const uint32_t height);

    Instance();
    ~Instance();

private:
    vk::UniqueInstance m_instance;

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
