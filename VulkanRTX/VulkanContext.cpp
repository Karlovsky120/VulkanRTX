#include "VulkanContext.h"

#include <GLFW/glfw3.h>

#include <iostream>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

void VulkanContext::createInstance() {
    if (!glfwRawMouseMotionSupported()) {
        throw std::runtime_error("Raw mouse motion not supported!");
    }

    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
        m_loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

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
        "VK_LAYER_KHRONOS_validation",
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
    VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_instance);

#ifdef ENABLE_VALIDATION
    m_debugMessenger = m_instance->createDebugUtilsMessengerEXT(debugCreateInfo);
#endif
}

void VulkanContext::createSurface(GLFWwindow* window) {
    VkSurfaceKHR vulkanSurface;
    if (glfwCreateWindowSurface(*m_instance, window, nullptr, &vulkanSurface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }

    m_surface = vk::UniqueSurfaceKHR(vulkanSurface, *m_instance);
}

void VulkanContext::createPhysicalDevice() {
    std::vector<vk::PhysicalDevice> physicalDevices = m_instance->enumeratePhysicalDevices();

    if (physicalDevices.size() == 0) {
        throw std::runtime_error("Failed to find GPU with Vulkan support!");
    }

    m_physicalDevice = physicalDevices[0];

    for (vk::PhysicalDevice currentPhysicalDevice : physicalDevices) {
        m_deviceProperties = m_physicalDevice.getProperties();
        m_deviceFeatures = m_physicalDevice.getFeatures();
        m_deviceMemoryProperties = m_physicalDevice.getMemoryProperties();

        std::vector<vk::ExtensionProperties> extensionProperties = m_physicalDevice.enumerateDeviceExtensionProperties();

        bool rayTracingSupported = false;
        for (vk::ExtensionProperties extensionProperty : extensionProperties) {
            if (extensionProperty.extensionName == VK_KHR_RAY_TRACING_EXTENSION_NAME) {
                rayTracingSupported = true;
                m_rayTracingProperties =
                    m_physicalDevice.getProperties2<vk::PhysicalDeviceProperties2,
                    vk::PhysicalDeviceRayTracingPropertiesKHR>().get<vk::PhysicalDeviceRayTracingPropertiesKHR>();
                break;
            }
        }

        if (m_deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            m_physicalDevice = currentPhysicalDevice;
            m_rayTracingSupported = rayTracingSupported;
            break;
        }
    }
}

void VulkanContext::createLogicalDevice() {
    auto queueFamilyProperties = m_physicalDevice.getQueueFamilyProperties();

    uint32_t queuesFound = 0;

    uint32_t i = 0;
    for (auto& queueFamilyProperty : queueFamilyProperties) {
        if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eGraphics) {
            m_graphicsQueueIndex = i;
            ++queuesFound;
            break;
        }
    }

    i = 0;
    for (auto& queueFamilyProperty : queueFamilyProperties) {
        if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eTransfer) {
            m_transferQueueIndex = i;
            ++queuesFound;
            break;
        }
    }

    i = 0;
    for (auto& queueFamilyProperty : queueFamilyProperties) {
        if (m_physicalDevice.getSurfaceSupportKHR(i, *m_surface)) {
            m_presentQueueIndex = i;
            ++queuesFound;
            break;
        }
    }

    if (queuesFound < 3) {
        throw std::runtime_error("Couldn't find suitable device queues!");
    }

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

    vk::DeviceQueueCreateInfo graphicsInfo;
    graphicsInfo.queueFamilyIndex = m_graphicsQueueIndex;
    graphicsInfo.queueCount = 1;
    float graphicsQueuePriority = 1.0f;
    graphicsInfo.pQueuePriorities = &graphicsQueuePriority;
    queueCreateInfos.push_back(graphicsInfo);

    if (m_graphicsQueueIndex != m_transferQueueIndex) {
        vk::DeviceQueueCreateInfo transferInfo;
        transferInfo.queueFamilyIndex = m_transferQueueIndex;
        transferInfo.queueCount = 1;
        float transferQueuePriority = 0.5f;
        transferInfo.pQueuePriorities = &transferQueuePriority;
        queueCreateInfos.push_back(transferInfo);
    }

    if (m_presentQueueIndex != m_graphicsQueueIndex && m_presentQueueIndex != m_transferQueueIndex) {
        vk::DeviceQueueCreateInfo presentInfo;
        presentInfo.queueFamilyIndex = m_presentQueueIndex;
        presentInfo.queueCount = 1;
        float presentQueuePriority = 0.75f;
        presentInfo.pQueuePriorities = &presentQueuePriority;
        queueCreateInfos.push_back(presentInfo);
    }

    vk::PhysicalDeviceTimelineSemaphoreFeatures timelineSemaphore;
    timelineSemaphore.timelineSemaphore = true;

    vk::PhysicalDeviceFeatures2 deviceFeatures;
    deviceFeatures.pNext = &timelineSemaphore;

    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pNext = &deviceFeatures;

    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    if (m_rayTracingSupported) {
        deviceExtensions.push_back(VK_KHR_RAY_TRACING_EXTENSION_NAME);
    }

    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

    deviceCreateInfo.pNext = &deviceFeatures;

    m_logicalDevice = m_physicalDevice.createDeviceUnique(deviceCreateInfo);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_logicalDevice);

    m_graphicsQueue = m_logicalDevice->getQueue(m_graphicsQueueIndex, 0);
    m_transferQueue = m_logicalDevice->getQueue(m_transferQueueIndex, 0);
    m_presentQueue = m_logicalDevice->getQueue(m_presentQueueIndex, 0);
}

void VulkanContext::initContext(GLFWwindow* window) {
    createInstance();
    createSurface(window);
    createPhysicalDevice();
    createLogicalDevice();
}

vk::UniqueDescriptorPool VulkanContext::createDescriptorPool(const uint32_t descriptorSetCount) const {
    vk::DescriptorPoolSize poolSize;
    poolSize.descriptorCount = descriptorSetCount;

    vk::DescriptorPoolCreateInfo poolInfo;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = descriptorSetCount;
    poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

    return  m_logicalDevice->createDescriptorPoolUnique(poolInfo);
}

vk::UniqueDescriptorSetLayout VulkanContext::createUniformDescriptorSetLayout() const {
    vk::DescriptorSetLayoutBinding uboLayoutBinding;
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    return m_logicalDevice->createDescriptorSetLayoutUnique(layoutInfo);
}

std::vector<vk::UniqueDescriptorSet> VulkanContext::createDescriptorSets(
    const vk::DescriptorPool& descriptorPool,
    const std::vector<vk::DescriptorSetLayout>& setLayouts) const {

    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(setLayouts.size());
    allocInfo.pSetLayouts = setLayouts.data();

    return  m_logicalDevice->allocateDescriptorSetsUnique(allocInfo);
}

vk::UniqueSemaphore VulkanContext::createTimelineSemaphore(const uint32_t initialValue) const {
    vk::SemaphoreTypeCreateInfo timelineCreateInfo;
    timelineCreateInfo.semaphoreType = vk::SemaphoreType::eTimeline;
    timelineCreateInfo.initialValue = initialValue;

    vk::SemaphoreCreateInfo createInfo;
    createInfo.pNext = &timelineCreateInfo;

    return m_logicalDevice->createSemaphoreUnique(createInfo);
}

#ifdef ENABLE_VALIDATION
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::cerr << pCallbackData->pMessage << std::endl;
    std::cerr << "----------------------------------------" << std::endl;

    return VK_FALSE;
}
#endif

VulkanContext::~VulkanContext() {
#ifdef ENABLE_VALIDATION
    m_instance->destroyDebugUtilsMessengerEXT(m_debugMessenger);
#endif
}