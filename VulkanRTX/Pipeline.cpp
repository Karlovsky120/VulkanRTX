#include "Pipeline.h"

#include <fstream>

void Pipeline::createPipeline(const vk::Extent2D& extent) {
	m_pipelineInfo.bindingDescription.binding = 0;
	m_pipelineInfo.bindingDescription.stride = sizeof(Vertex);
	m_pipelineInfo.bindingDescription.inputRate = vk::VertexInputRate::eVertex;

	m_pipelineInfo.attributeDescriptions = std::vector<vk::VertexInputAttributeDescription>(2);

	m_pipelineInfo.attributeDescriptions[0].binding = 0;
	m_pipelineInfo.attributeDescriptions[0].location = 0;
	m_pipelineInfo.attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
	m_pipelineInfo.attributeDescriptions[0].offset = offsetof(Vertex, m_pos);

	m_pipelineInfo.attributeDescriptions[1].binding = 0;
	m_pipelineInfo.attributeDescriptions[1].location = 1;
	m_pipelineInfo.attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
	m_pipelineInfo.attributeDescriptions[1].offset = offsetof(Vertex, m_color);

	m_pipelineInfo.vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
	m_pipelineInfo.vertexInputCreateInfo.vertexAttributeDescriptionCount = m_pipelineInfo.attributeDescriptions.size();
	m_pipelineInfo.vertexInputCreateInfo.pVertexBindingDescriptions = &m_pipelineInfo.bindingDescription;
	m_pipelineInfo.vertexInputCreateInfo.pVertexAttributeDescriptions = m_pipelineInfo.attributeDescriptions.data();

	m_pipelineInfo.inputAssemblyCreateInfo.topology = vk::PrimitiveTopology::eTriangleList;
	m_pipelineInfo.inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	m_pipelineInfo.viewport.x = 0.0f;
	m_pipelineInfo.viewport.y = 0.0f;
	m_pipelineInfo.viewport.width = (float)extent.width;
	m_pipelineInfo.viewport.height = (float)extent.height;
	m_pipelineInfo.viewport.minDepth = 0.0f;
	m_pipelineInfo.viewport.maxDepth = 1.0f;

	m_pipelineInfo.scissor.offset = vk::Offset2D(0, 0);
	m_pipelineInfo.scissor.extent = extent;

	m_pipelineInfo.viewportCreateInfo.viewportCount = 1;
	m_pipelineInfo.viewportCreateInfo.pViewports = &m_pipelineInfo.viewport;
	m_pipelineInfo.viewportCreateInfo.scissorCount = 1;
	m_pipelineInfo.viewportCreateInfo.pScissors = &m_pipelineInfo.scissor;

	m_pipelineInfo.rasterizerationCreateInfo.depthClampEnable = VK_FALSE;
	m_pipelineInfo.rasterizerationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	m_pipelineInfo.rasterizerationCreateInfo.polygonMode = vk::PolygonMode::eFill;
	m_pipelineInfo.rasterizerationCreateInfo.lineWidth = 1.0f;
	m_pipelineInfo.rasterizerationCreateInfo.cullMode = vk::CullModeFlagBits::eBack;
	m_pipelineInfo.rasterizerationCreateInfo.frontFace = vk::FrontFace::eClockwise;
	m_pipelineInfo.rasterizerationCreateInfo.depthBiasEnable = VK_FALSE;
	m_pipelineInfo.rasterizerationCreateInfo.depthBiasConstantFactor = 0.0f;
	m_pipelineInfo.rasterizerationCreateInfo.depthBiasClamp = 0.0f;
	m_pipelineInfo.rasterizerationCreateInfo.depthBiasSlopeFactor = 0.0f;

	m_pipelineInfo.multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
	m_pipelineInfo.multisampleCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
	m_pipelineInfo.multisampleCreateInfo.minSampleShading = 1.0f;
	m_pipelineInfo.multisampleCreateInfo.pSampleMask = nullptr;
	m_pipelineInfo.multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
	m_pipelineInfo.multisampleCreateInfo.alphaToOneEnable = VK_FALSE;

	m_pipelineInfo.colorBlendAttachment.colorWriteMask =
		vk::ColorComponentFlagBits::eR |
		vk::ColorComponentFlagBits::eG |
		vk::ColorComponentFlagBits::eB |
		vk::ColorComponentFlagBits::eA;
	m_pipelineInfo.colorBlendAttachment.blendEnable = VK_FALSE;
	m_pipelineInfo.colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
	m_pipelineInfo.colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
	m_pipelineInfo.colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
	m_pipelineInfo.colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
	m_pipelineInfo.colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
	m_pipelineInfo.colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

	m_pipelineInfo.colorBlendCreateInfo.logicOpEnable = VK_FALSE;
	m_pipelineInfo.colorBlendCreateInfo.logicOp = vk::LogicOp::eCopy;
	m_pipelineInfo.colorBlendCreateInfo.attachmentCount = 1;
	m_pipelineInfo.colorBlendCreateInfo.pAttachments = &m_pipelineInfo.colorBlendAttachment;
	m_pipelineInfo.colorBlendCreateInfo.blendConstants[0] = 0.0f;
	m_pipelineInfo.colorBlendCreateInfo.blendConstants[1] = 0.0f;
	m_pipelineInfo.colorBlendCreateInfo.blendConstants[2] = 0.0f;
	m_pipelineInfo.colorBlendCreateInfo.blendConstants[3] = 0.0f;

	m_pipelineInfo.vertexShaderModule = createShaderModule("shaders/bin/vertexShader.vert.spv");
	m_pipelineInfo.vertShaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eVertex;
	m_pipelineInfo.vertShaderStageCreateInfo.module = *m_pipelineInfo.vertexShaderModule;
	m_pipelineInfo.vertShaderStageCreateInfo.pName = "main";

	m_pipelineInfo.fragmentShaderModule = createShaderModule("shaders/bin/fragmentShader.frag.spv");
	m_pipelineInfo.fragShaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eFragment;
	m_pipelineInfo.fragShaderStageCreateInfo.module = *m_pipelineInfo.fragmentShaderModule;
	m_pipelineInfo.fragShaderStageCreateInfo.pName = "main";

	m_pipelineCache = m_logicalDevice.createPipelineCacheUnique(m_cacheCreateInfo);

	m_pipelineInfo.shaderStages = { m_pipelineInfo.vertShaderStageCreateInfo, m_pipelineInfo.fragShaderStageCreateInfo };

	m_pipelineInfo.pipelineCreateInfo.stageCount = 2;
	m_pipelineInfo.pipelineCreateInfo.pStages = m_pipelineInfo.shaderStages.data();

	m_pipelineInfo.pipelineCreateInfo.pVertexInputState = &m_pipelineInfo.vertexInputCreateInfo;
	m_pipelineInfo.pipelineCreateInfo.pInputAssemblyState = &m_pipelineInfo.inputAssemblyCreateInfo;
	m_pipelineInfo.pipelineCreateInfo.pViewportState = &m_pipelineInfo.viewportCreateInfo;
	m_pipelineInfo.pipelineCreateInfo.pRasterizationState = &m_pipelineInfo.rasterizerationCreateInfo;
	m_pipelineInfo.pipelineCreateInfo.pMultisampleState = &m_pipelineInfo.multisampleCreateInfo;
	m_pipelineInfo.pipelineCreateInfo.pDepthStencilState = nullptr;
	m_pipelineInfo.pipelineCreateInfo.pColorBlendState = &m_pipelineInfo.colorBlendCreateInfo;
	m_pipelineInfo.pipelineCreateInfo.pDynamicState = nullptr;

	m_pipelineInfo.pipelineCreateInfo.layout = *m_pipelineLayout;

	m_pipelineInfo.pipelineCreateInfo.flags = vk::PipelineCreateFlagBits::eAllowDerivatives;
	m_pipelineInfo.pipelineCreateInfo.renderPass = *m_renderPass;
	m_pipelineInfo.pipelineCreateInfo.subpass = 0;

	m_pipelineInfo.pipelineCreateInfo.basePipelineHandle = nullptr;
	m_pipelineInfo.pipelineCreateInfo.basePipelineIndex = -1;

	m_pipeline = m_logicalDevice.createGraphicsPipelineUnique(*m_pipelineCache, m_pipelineInfo.pipelineCreateInfo);

	m_pipelineInfo.pipelineCreateInfo.flags |= vk::PipelineCreateFlagBits::eDerivative;
	m_pipelineInfo.pipelineCreateInfo.basePipelineIndex = 0;

}

void Pipeline::createRenderPass(const vk::Format& format) {
	m_renderPassInfo.colorAttachment.format = format;
	m_renderPassInfo.colorAttachment.samples = vk::SampleCountFlagBits::e1;

	m_renderPassInfo.colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	m_renderPassInfo.colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	m_renderPassInfo.colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	m_renderPassInfo.colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

	m_renderPassInfo.colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	m_renderPassInfo.colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	m_renderPassInfo.colorAttachmentRef.attachment = 0;
	m_renderPassInfo.colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	m_renderPassInfo.subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	m_renderPassInfo.subpass.colorAttachmentCount = 1;
	m_renderPassInfo.subpass.pColorAttachments = &m_renderPassInfo.colorAttachmentRef;

	m_renderPassInfo.dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	m_renderPassInfo.dependency.dstSubpass = 0;
	m_renderPassInfo.dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	m_renderPassInfo.dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	m_renderPassInfo.dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

	m_renderPassInfo.renderPassCreateInfo.attachmentCount = 1;
	m_renderPassInfo.renderPassCreateInfo.pAttachments = &m_renderPassInfo.colorAttachment;
	m_renderPassInfo.renderPassCreateInfo.subpassCount = 1;
	m_renderPassInfo.renderPassCreateInfo.pSubpasses = &m_renderPassInfo.subpass;
	m_renderPassInfo.renderPassCreateInfo.dependencyCount = 1;
	m_renderPassInfo.renderPassCreateInfo.pDependencies = &m_renderPassInfo.dependency;

	m_renderPass = m_logicalDevice.createRenderPassUnique(m_renderPassInfo.renderPassCreateInfo);
}

void Pipeline::createPipelineLayout(const vk::DescriptorSetLayout* setLayout) {
	std::vector<vk::PushConstantRange> pushConstantRanges;

	vk::PushConstantRange cameraMatrix;
	cameraMatrix.stageFlags = vk::ShaderStageFlagBits::eVertex;
	cameraMatrix.offset = 0;
	cameraMatrix.size = 16 * sizeof(float);

	pushConstantRanges.push_back(cameraMatrix);

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = setLayout;
	pipelineLayoutInfo.pushConstantRangeCount = pushConstantRanges.size();
	pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

	m_pipelineLayout = m_logicalDevice.createPipelineLayoutUnique(pipelineLayoutInfo);
}

void Pipeline::initPipeline(
	const vk::Extent2D& extent,
	const vk::Format& format,
	const vk::DescriptorSetLayout* setLayout) {

	createRenderPass(format);
	createPipelineLayout(setLayout);
	createPipeline(extent);
}

void Pipeline::updatePipeline(const vk::Format& format, const vk::Extent2D& extent) {
	if (format != m_renderPassInfo.colorAttachment.format) {
		m_logicalDevice.destroyRenderPass(*m_renderPass);

		m_renderPassInfo.colorAttachment.format = format;
		m_renderPass = m_logicalDevice.createRenderPassUnique(m_renderPassInfo.renderPassCreateInfo);
	}

	m_pipelineInfo.viewport.width = (float)extent.width;
	m_pipelineInfo.viewport.height = (float)extent.height;
	m_pipelineInfo.scissor.extent = extent;

	vk::Pipeline oldPipeline = *m_pipeline;
	m_pipelineInfo.pipelineCreateInfo.basePipelineHandle = oldPipeline;

	m_pipeline = m_logicalDevice.createGraphicsPipelineUnique(*m_pipelineCache, m_pipelineInfo.pipelineCreateInfo);
}

vk::UniqueShaderModule Pipeline::createShaderModule(const std::string shaderPath) const {
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

	return m_logicalDevice.createShaderModuleUnique(createInfo);
}

Pipeline::Pipeline(vk::Device& logicalDevice) :
	m_logicalDevice(logicalDevice) {}