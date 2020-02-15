#include "CommandBuffer.h"

#include "Swapchain.h"

void CommandBuffer::begin() {
    vk::CommandBufferBeginInfo beginInfo;

    commandBuffer.begin(beginInfo);
}

void CommandBuffer::beginRenderPass(vk::RenderPass& renderPass, vk::Framebuffer& framebuffer, Swapchain& swapchain) {
    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffer;

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapchain.getExtent();

    vk::ClearValue clearColor = vk::ClearColorValue(std::array<float, 4>({0.0f, 0.0f, 0.0f, 1.0f}));
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
}

CommandBuffer::CommandBuffer(vk::Device& device, vk::CommandPool& commandPool) :
    device(device) {

    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.commandPool = commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    commandBuffer = device.allocateCommandBuffers(allocInfo)[0];
}