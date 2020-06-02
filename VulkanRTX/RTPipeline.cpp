#include "RTPipeline.h"

#include "VulkanContext.h"

#include <fstream>

void RTPipeline::createPipeline(vk::DescriptorSetLayout& setLayout) {
	vk::PipelineCacheCreateInfo cacheCreateInfo;
	cacheCreateInfo.initialDataSize = 0;

	m_pipelineCache = VulkanContext::getDevice().createPipelineCacheUnique(cacheCreateInfo);

	vk::PushConstantRange viewInverse;
	viewInverse.stageFlags =
		vk::ShaderStageFlagBits::eRaygenKHR
		| vk::ShaderStageFlagBits::eClosestHitKHR;
	viewInverse.offset = 0;
	viewInverse.size = 16 * sizeof(float);

	vk::PushConstantRange projInverse;
	projInverse.stageFlags =
		vk::ShaderStageFlagBits::eRaygenKHR
		| vk::ShaderStageFlagBits::eClosestHitKHR;
	projInverse.offset = 16 * sizeof(float);
	projInverse.size = 16 * sizeof(float);

	vk::PushConstantRange playerPosition;
	playerPosition.stageFlags =
		vk::ShaderStageFlagBits::eRaygenKHR
		| vk::ShaderStageFlagBits::eClosestHitKHR;
	playerPosition.offset = 32 * sizeof(float);
	playerPosition.size = 3 * sizeof(float);

	vk::PushConstantRange lightPosition;
	lightPosition.stageFlags =
		vk::ShaderStageFlagBits::eRaygenKHR
		| vk::ShaderStageFlagBits::eClosestHitKHR;
	lightPosition.offset = 35 * sizeof(float);
	lightPosition.size = 3 * sizeof(float);

	std::vector<vk::PushConstantRange> pushConstantRanges = {
		viewInverse,
		projInverse,
		playerPosition,
		lightPosition };

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &setLayout;
	pipelineLayoutInfo.pushConstantRangeCount = pushConstantRanges.size();
	pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

	m_pipelineLayout = VulkanContext::getDevice().createPipelineLayoutUnique(pipelineLayoutInfo);

	std::array<vk::PipelineShaderStageCreateInfo, 4> stageInfos;

	vk::PipelineShaderStageCreateInfo raygenShaderInfo;
	vk::UniqueShaderModule raygenModule =
		createShaderModule("shaders/bin/raygenShader.rgen.spv");
	raygenShaderInfo.module = *raygenModule;
	raygenShaderInfo.stage = vk::ShaderStageFlagBits::eRaygenKHR;
	raygenShaderInfo.pName = "main";
	stageInfos[INDEX_RAYGEN] = raygenShaderInfo;

	vk::PipelineShaderStageCreateInfo missShaderInfo;
	vk::UniqueShaderModule missModule =
		createShaderModule("shaders/bin/missShader.rmiss.spv");
	missShaderInfo.module = *missModule;
	missShaderInfo.stage = vk::ShaderStageFlagBits::eMissKHR;
	missShaderInfo.pName = "main";
	stageInfos[INDEX_MISS] = missShaderInfo;

	vk::PipelineShaderStageCreateInfo shadowMissShaderInfo;
	vk::UniqueShaderModule shadowMissModule =
		createShaderModule("shaders/bin/shadowMissShader.rmiss.spv");
	shadowMissShaderInfo.module = *shadowMissModule;
	shadowMissShaderInfo.stage = vk::ShaderStageFlagBits::eMissKHR;
	shadowMissShaderInfo.pName = "main";
	stageInfos[INDEX_SHADOW_MISS] = shadowMissShaderInfo;


	vk::PipelineShaderStageCreateInfo closestHitShaderInfo;
	vk::UniqueShaderModule closestHitModule =
		createShaderModule("shaders/bin/closestHitShader.rchit.spv");
	closestHitShaderInfo.module = *closestHitModule;
	closestHitShaderInfo.stage = vk::ShaderStageFlagBits::eClosestHitKHR;
	closestHitShaderInfo.pName = "main";
	stageInfos[INDEX_CLOSEST_HIT] = closestHitShaderInfo;

	std::array<vk::RayTracingShaderGroupCreateInfoKHR, 4> groupInfos;
	for (auto& group : groupInfos) {
		group.generalShader = VK_SHADER_UNUSED_KHR;
		group.closestHitShader = VK_SHADER_UNUSED_KHR;
		group.anyHitShader = VK_SHADER_UNUSED_KHR;
		group.intersectionShader = VK_SHADER_UNUSED_KHR;
	}

	groupInfos[INDEX_RAYGEN].type = vk::RayTracingShaderGroupTypeKHR::eGeneral;
	groupInfos[INDEX_RAYGEN].generalShader = INDEX_RAYGEN;
	groupInfos[INDEX_MISS].type = vk::RayTracingShaderGroupTypeKHR::eGeneral;
	groupInfos[INDEX_MISS].generalShader = INDEX_MISS;
	groupInfos[INDEX_SHADOW_MISS].type = vk::RayTracingShaderGroupTypeKHR::eGeneral;
	groupInfos[INDEX_SHADOW_MISS].generalShader = INDEX_SHADOW_MISS;
	groupInfos[INDEX_CLOSEST_HIT].type = vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup;
	groupInfos[INDEX_CLOSEST_HIT].closestHitShader = INDEX_CLOSEST_HIT;

	vk::RayTracingPipelineCreateInfoKHR pipelineCreateInfo;
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(stageInfos.size());
	pipelineCreateInfo.pStages = stageInfos.data();
	pipelineCreateInfo.groupCount = static_cast<uint32_t>(groupInfos.size());
	pipelineCreateInfo.pGroups = groupInfos.data();
	pipelineCreateInfo.maxRecursionDepth = 1;
	pipelineCreateInfo.layout = *m_pipelineLayout;

	m_pipeline = VulkanContext::getDevice().createRayTracingPipelineKHRUnique(*m_pipelineCache, pipelineCreateInfo).value;
}

vk::Pipeline& RTPipeline::get() {
	return *m_pipeline;
}

vk::PipelineLayout& RTPipeline::getLayout() {
	return *m_pipelineLayout;
}

vk::UniqueShaderModule RTPipeline::createShaderModule(
	const std::string shaderPath) const {

	std::ifstream shaderFile(shaderPath, std::ios::ate | std::ios::binary);

	if (!shaderFile.is_open()) {
		throw std::runtime_error("Failed to open file " + shaderPath + "!");
	}

	size_t fileSize = (size_t)shaderFile.tellg();

	std::vector<char> code(fileSize);

	shaderFile.seekg(0);
	shaderFile.read(code.data(), fileSize);

	shaderFile.close();

	vk::ShaderModuleCreateInfo createInfo;
	createInfo.codeSize = fileSize;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	return VulkanContext::getDevice().createShaderModuleUnique(createInfo);
}