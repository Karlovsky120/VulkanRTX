#include "CommandBuffer.h"

#include "CommandPool.h"
#include "LogicalDevice.h"
#include "Swapchain.h"

vk::CommandBuffer& CommandBuffer::get() {
    return m_commandBuffer;
}

void CommandBuffer::begin() {
    vk::CommandBufferBeginInfo beginInfo;

    m_commandBuffer.begin(beginInfo);
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

    m_commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
}

CommandBuffer::CommandBuffer(LogicalDevice& logicalDevice, CommandPool& commandPool) :
    m_logicalDevice(logicalDevice) {

    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.commandPool = commandPool.get();
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    m_commandBuffer = logicalDevice.get().allocateCommandBuffers(allocInfo)[0];
}