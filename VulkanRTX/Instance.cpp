#include "Instance.h"

#include "PhysicalDevice.h"
#include "Surface.h"

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <iostream>

vk::Instance& Instance::get() {
    return *m_instance;
}

std::unique_ptr<PhysicalDevice> Instance::createPhysicalDevice() {
    return std::make_unique<PhysicalDevice>(*m_instance);
}

std::unique_ptr<Surface> Instance::createSurface(GLFWwindow* window,
                                                 const uint32_t width,
                                                 const uint32_t height) {

    return std::make_unique<Surface>(*m_instance, window, width, height);
}

Instance::Instance() {
    vk::ApplicationInfo appInfo;
    appInfo.pApplicationName = "RTX Application";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName = "Nengine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion = VK_API_VERSION_1_1;

    vk::InstanceCreateInfo createInfo;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef ENABLE_VALIDATION
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

#ifdef ENABLE_VALIDATION
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_LUNARG_api_dump",
        "VK_LAYER_KHRONOS_validation"
    };

    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();

    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    debugCreateInfo.messageSeverity =
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    debugCreateInfo.messageType =
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    debugCreateInfo.pfnUserCallback = debugCallback;

    createInfo.pNext = &debugCreateInfo;
#else
    createInfo.enabledLayerCount = 0;
#endif

    m_instance = vk::createInstanceUnique(createInfo);
    m_loader = vk::DispatchLoaderDynamic(*m_instance, vkGetInstanceProcAddr);

#ifdef ENABLE_VALIDATION
    m_debugMessenger = m_instance->createDebugUtilsMessengerEXT(debugCreateInfo, nullptr, m_loader);
#endif
}

#ifdef ENABLE_VALIDATION
VKAPI_ATTR VkBool32 VKAPI_CALL Instance::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::cerr << pCallbackData->pMessage << std::endl;
    std::cerr << "----------------------------------------" << std::endl;

    return VK_FALSE;
}
#endif

Instance::~Instance() {
#ifdef ENABLE_VALIDATION
    m_instance->destroyDebugUtilsMessengerEXT(m_debugMessenger, nullptr, m_loader);
#endif
}