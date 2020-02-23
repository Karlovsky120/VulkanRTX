#include "ShaderModule.h"

#include <fstream>
#include <vector>

vk::ShaderModule& ShaderModule::get() {
    return *m_shaderModule;
}

ShaderModule::ShaderModule(vk::Device& logicalDevice, const std::string shaderPath) :
    m_logicalDevice(logicalDevice) {

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

    m_shaderModule = logicalDevice.createShaderModuleUnique(createInfo);
}