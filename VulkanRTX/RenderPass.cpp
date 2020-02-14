#include "RenderPass.h"

#include "Swapchain.h"

vk::RenderPass& RenderPass::getRenderPass() {
    return renderPass;
}

RenderPass::RenderPass(vk::Device& device, Swapchain& swapchain) :
    device(device) {

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

    renderPass = device.createRenderPass(renderPassInfo);
}

RenderPass::~RenderPass() {
    device.destroyRenderPass(renderPass);
}