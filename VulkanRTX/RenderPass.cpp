#include "RenderPass.h"

#include "LogicalDevice.h"
#include "Swapchain.h"

vk::RenderPass& RenderPass::get() {
    return m_renderPass;
}

RenderPass::RenderPass(LogicalDevice& logicalDevice, Swapchain& swapchain) :
    m_logicalDevice(logicalDevice) {

    vk::AttachmentDescription colorAttachment;
    colorAttachment.format = swapchain.getFormat().format;
    colorAttachment.samples = vk::SampleCountFlagBits::e1;

    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

    colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpass;
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    m_renderPass = logicalDevice.get().createRenderPass(renderPassInfo);
}

RenderPass::~RenderPass() {
    m_logicalDevice.get().destroyRenderPass(m_renderPass);
}