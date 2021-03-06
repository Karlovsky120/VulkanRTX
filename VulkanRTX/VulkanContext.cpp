#include "VulkanContext.h"

#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

void VulkanContext::init(std::shared_ptr<VulkanContext>& storage, GLFWwindow* window) {
    storage = std::make_shared<VulkanContext>(window);
    m_staticInstance = storage;
}

std::shared_ptr<VulkanContext> VulkanContext::get() {
    return m_staticInstance.lock();
}

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
    appInfo.apiVersion = VK_API_VERSION_1_2;

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
#ifdef ENABLE_API_DUMP
        "VK_LAYER_LUNARG_api_dump",
#endif
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
        bool pipelineLibrarySupported = false;
        bool deferredHostOperationsSupported = false;
        bool bufferDeviceAddressSUpported = false;
        bool descriptorIndexingSupported = false;
#ifdef AFTERMATH
        bool aftermathCheckpointsSupported = false;
        bool aftermathConfigSupported = false;
#endif
#ifdef OPTIX_DENOISER
        bool externalMemorySupported = false;
#endif

        bool deviceFound = false;
        for (vk::ExtensionProperties extensionProperty : extensionProperties) {
            std::string extensionName(extensionProperty.extensionName);

            if (extensionName == VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) {
                bufferDeviceAddressSUpported = true;
            }

            else if (extensionName == VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME) {
                deferredHostOperationsSupported = true;
            }

            else if (extensionName == VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME) {
                pipelineLibrarySupported = true;
            }

            else if (extensionName == VK_KHR_RAY_TRACING_EXTENSION_NAME) {
                rayTracingSupported = true;
                m_rayTracingProperties =
                    m_physicalDevice.getProperties2<vk::PhysicalDeviceProperties2,
                    vk::PhysicalDeviceRayTracingPropertiesKHR>().get<vk::PhysicalDeviceRayTracingPropertiesKHR>();
            }

            else if (extensionName == VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME) {
                descriptorIndexingSupported = true;
            }

#ifdef AFTERMATH
            else if (extensionName == VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME) {
                aftermathCheckpointsSupported = true;
            }

            else if (extensionName == VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME) {
                aftermathConfigSupported = true;
            }
#endif
#ifdef OPTIX_DENOISER
            else if (extensionName == VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME) {
                externalMemorySupported = true;
            }
#endif
#ifdef ENABLE_DEBUG_MARKERS
            else if (extensionName == VK_EXT_DEBUG_MARKER_EXTENSION_NAME) {
                m_debugMarkersSupported = true;
            }
#endif
        }

        if (m_deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            m_physicalDevice = currentPhysicalDevice;

            if (rayTracingSupported
                && pipelineLibrarySupported
                && deferredHostOperationsSupported
                && bufferDeviceAddressSUpported
                && descriptorIndexingSupported
#ifdef OPTIX_DENOISER
                && externalMemorySupported
#endif
                ) {

#ifdef AFTERMATH
                m_aftermathSupported =
                    aftermathCheckpointsSupported
                    && aftermathConfigSupported;
#endif
                deviceFound = true;
                break;
            }
        }

        if (!deviceFound) {
            throw std::runtime_error("No suitable physical device found!");
        }
    }
}

uint32_t VulkanContext::findTransferQueue(std::vector<vk::QueueFamilyProperties> queueFamilyProperties) {
    uint32_t candidate = -1;

    uint32_t index = 0;
    for (auto& queueFamilyProperty : queueFamilyProperties) {
        if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eTransfer) {
            if (!(queueFamilyProperty.queueFlags
                & vk::QueueFlagBits::eGraphics
                && queueFamilyProperty.queueFlags
                & vk::QueueFlagBits::eCompute)) {
                return index;
            }
            candidate = index;
        }
        ++index;
    }

    return candidate;
}

uint32_t VulkanContext::findComputeQueue(std::vector<vk::QueueFamilyProperties> queueFamilyProperties) {
    uint32_t candidate = -1;

    uint32_t index = 0;
    for (auto& queueFamilyProperty : queueFamilyProperties) {
        if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eCompute) {
            if (!(queueFamilyProperty.queueFlags
                & vk::QueueFlagBits::eGraphics)) {
                return index;
            }
            candidate = index;
        }
        ++index;
    }

    return candidate;
}

uint32_t VulkanContext::findGraphicsQueue(std::vector<vk::QueueFamilyProperties> queueFamilyProperties) {
    uint32_t index = 0;
    for (auto& queueFamilyProperty : queueFamilyProperties) {
        if (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eGraphics) {
            return index;
        }
        ++index;
    }

    return -1;
}

uint32_t VulkanContext::findPresentQueue(uint32_t count, vk::SurfaceKHR surface, vk::PhysicalDevice physicalDevice) {
    for (uint32_t i = 0; i < count; ++i) {
        if (physicalDevice.getSurfaceSupportKHR(i, surface)) {
            return i;
        }
    }

    return -1;
}

void VulkanContext::createLogicalDevice() {
    auto queueFamilyProperties = m_physicalDevice.getQueueFamilyProperties();

    m_graphicsQueueIndex = findGraphicsQueue(queueFamilyProperties);
    if (m_graphicsQueueIndex == -1) {
        throw std::runtime_error("Couldn't find graphics queue!");
    }

    m_computeQueueIndex = findComputeQueue(queueFamilyProperties);
    if (m_computeQueueIndex == -1) {
        throw std::runtime_error("Couldn't find compute queue!");
    }

    m_transferQueueIndex = findTransferQueue(queueFamilyProperties);
    if (m_transferQueueIndex == -1) {
        throw std::runtime_error("Couldn't find transfer queue!");
    }

    m_presentQueueIndex = findPresentQueue(static_cast<uint32_t>(queueFamilyProperties.size()), *m_surface, m_physicalDevice);
    if (m_presentQueueIndex == -1) {
        throw std::runtime_error("Couldn't find present queue!");
    }

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

    vk::DeviceQueueCreateInfo graphicsInfo;
    graphicsInfo.queueFamilyIndex = m_graphicsQueueIndex;
    graphicsInfo.queueCount = 1;
    float graphicsQueuePriority = 1.0f;
    graphicsInfo.pQueuePriorities = &graphicsQueuePriority;
    queueCreateInfos.push_back(graphicsInfo);

    if (m_computeQueueIndex != m_graphicsQueueIndex) {

        vk::DeviceQueueCreateInfo computeInfo;
        computeInfo.queueFamilyIndex = m_computeQueueIndex;
        computeInfo.queueCount = 1;
        float computeQueuePriority = 0.66f;
        computeInfo.pQueuePriorities = &computeQueuePriority;
        queueCreateInfos.push_back(computeInfo);
    }

    if (m_transferQueueIndex != m_graphicsQueueIndex
        && m_transferQueueIndex != m_computeQueueIndex) {

        vk::DeviceQueueCreateInfo transferInfo;
        transferInfo.queueFamilyIndex = m_transferQueueIndex;
        transferInfo.queueCount = 1;
        float transferQueuePriority = 0.33f;
        transferInfo.pQueuePriorities = &transferQueuePriority;
        queueCreateInfos.push_back(transferInfo);
    }

    if (m_presentQueueIndex != m_graphicsQueueIndex
        && m_presentQueueIndex != m_computeQueueIndex
        && m_presentQueueIndex != m_transferQueueIndex) {

        vk::DeviceQueueCreateInfo presentInfo;
        presentInfo.queueFamilyIndex = m_presentQueueIndex;
        presentInfo.queueCount = 1;
        float presentQueuePriority = 0.75f;
        presentInfo.pQueuePriorities = &presentQueuePriority;
        queueCreateInfos.push_back(presentInfo);
    }
    
    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_RAY_TRACING_EXTENSION_NAME,
        VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
#ifdef OPTIX_DENOISER
        VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
#endif
    };

    void* currentFeature = nullptr;

    vk::PhysicalDeviceRayTracingFeaturesKHR rayTracing;
    rayTracing.rayTracing = true;
    currentFeature = &rayTracing;

    // Required for ray tracing
    vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR deviceAddress;
    deviceAddress.bufferDeviceAddress = true;
    deviceAddress.pNext = currentFeature;
    currentFeature = &deviceAddress;

    // Nice, but currently unused
    vk::PhysicalDeviceTimelineSemaphoreFeatures timelineSemaphore;
    timelineSemaphore.timelineSemaphore = true;
    timelineSemaphore.pNext = currentFeature;
    currentFeature = &timelineSemaphore;

#ifdef AFTERMATH
    if (m_aftermathSupported) {
        vk::DeviceDiagnosticsConfigCreateInfoNV diagnosticInfo;
        diagnosticInfo.flags =
            vk::DeviceDiagnosticsConfigFlagBitsNV::eEnableResourceTracking
            | vk::DeviceDiagnosticsConfigFlagBitsNV::eEnableAutomaticCheckpoints
            | vk::DeviceDiagnosticsConfigFlagBitsNV::eEnableShaderDebugInfo;

        diagnosticInfo.pNext = feature;
        currentFeature = &diagnosticInfo;

        deviceExtensions.push_back(VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME);
        deviceExtensions.push_back(VK_NV_DEVICE_DIAGNOSTICS_CONFIG_EXTENSION_NAME);

        m_gpuCrashTracker.Initialize();
    }
#endif

    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());   
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    deviceCreateInfo.pNext = currentFeature;

    m_logicalDevice = m_physicalDevice.createDeviceUnique(deviceCreateInfo);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_logicalDevice);

    m_graphicsQueue = m_logicalDevice->getQueue(m_graphicsQueueIndex, 0);
    m_computeQueue = m_logicalDevice->getQueue(m_computeQueueIndex, 0);
    m_transferQueue = m_logicalDevice->getQueue(m_transferQueueIndex, 0);
    m_presentQueue = m_logicalDevice->getQueue(m_presentQueueIndex, 0);
}

vk::Device& VulkanContext::getDevice() {
    return *m_staticInstance.lock()->m_logicalDevice;
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

#ifdef ENABLE_DEBUG_MARKERS
void VulkanContext::nameObject(void* object, vk::DebugReportObjectTypeEXT type, std::string name) {
    vk::DebugMarkerObjectNameInfoEXT objectNameInfo;
    objectNameInfo.objectType = type;
    objectNameInfo.pObjectName = name.c_str();

    uint64_t* objectPtr = reinterpret_cast<uint64_t*>(object);
    objectNameInfo.object = *objectPtr;

    m_staticInstance.lock()->m_logicalDevice->debugMarkerSetObjectNameEXT(objectNameInfo);
}
#endif

#ifdef ENABLE_VALIDATION
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::cerr << pCallbackData->pMessage << std::endl;
    std::cerr << "------------------------------------------------------------------------------------------------" << std::endl;

    return VK_FALSE;
}
#endif

VulkanContext::VulkanContext(GLFWwindow* window) {
    initContext(window);
}

std::weak_ptr<VulkanContext> VulkanContext::m_staticInstance;

#ifdef ENABLE_VALIDATION
VulkanContext::~VulkanContext() {
    m_instance->destroyDebugUtilsMessengerEXT(m_debugMessenger);
}
#endif