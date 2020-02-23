#include "CommandBuffer.h"

#include "CommandPool.h"
#include "LogicalDevice.h"
#include "Swapchain.h"

vk::CommandBuffer& CommandBuffer::get() {
    return *m_commandBuffer;
}

void CommandBuffer::begin() {
    vk::CommandBufferBeginInfo beginInfo;

    m_commandBuffer->begin(beginInfo);
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

    m_commandBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
}

CommandBuffer::CommandBuffer(vk::Device& logicalDevice, vk::CommandPool& commandPool) :
    m_logicalDevice(logicalDevice),
    m_commandPool(commandPool) {

    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.commandPool = commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;

    m_commandBuffer = std::move(logicalDevice.allocateCommandBuffersUnique(allocInfo)[0]);
}

void CommandBuffer::submit(vk::Queue& submitQueue,
                           vk::Fence& fence,
                           std::vector<vk::Semaphore> waitSemaphores,
                           std::vector<vk::Semaphore> signalSemaphores) {

    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &*m_commandBuffer;
    submitInfo.waitSemaphoreCount = waitSemaphores.size();
    submitInfo.pWaitSemaphores = &waitSemaphores[0];
    submitInfo.signalSemaphoreCount = signalSemaphores.size();
    submitInfo.pSignalSemaphores = &signalSemaphores[0];

    submitQueue.submit(submitInfo, fence);
}