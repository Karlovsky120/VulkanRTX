#include "VulkanShaderModule.h"

#include <fstream>
#include <vector>

VulkanShaderModule::VulkanShaderModule(vk::Device device, const std::string& shaderPath) :
    device(device) {

    std::ifstream file(shaderPath, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file " + shaderPath + "!");
    }

    size_t fileSize = (size_t)file.tellg();

    std::vector<char> code(fileSize);

    file.seekg(0);
    file.read(code.data(), fileSize);

    file.close();

    vk::ShaderModuleCreateInfo createInfo;
    createInfo.codeSize = fileSize;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    module = device.createShaderModule(createInfo);
}

VulkanShaderModule::~VulkanShaderModule() {
    device.destroyShaderModule(module);
}