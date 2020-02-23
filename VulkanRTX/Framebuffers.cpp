#include "Framebuffers.h"

#include "LogicalDevice.h"
#include "RenderPass.h"
#include "Swapchain.h"

vk::Framebuffer& Framebuffers::get(uint32_t index) {
    return *m_framebuffers[index];
}

vk::Framebuffer& Framebuffers::getNext() {
    return *m_framebuffers[m_currentIndex++];
}

Framebuffers::Framebuffers(vk::Device& logicalDevice, vk::RenderPass& renderPass, Swapchain& swapchain) :
    m_logicalDevice(logicalDevice) {
    std::vector<vk::ImageView> swapchainImageViews = swapchain.getImageViews();

    m_framebuffers.resize(swapchainImageViews.size());

    for (size_t i = 0; i < swapchainImageViews.size(); ++i) {
        vk::ImageView attachments[] = {
            swapchainImageViews[i]
        };

        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchain.getExtent().width;
        framebufferInfo.height = swapchain.getExtent().height;
        framebufferInfo.layers = 1;

        m_framebuffers[i] = m_logicalDevice.createFramebufferUnique(framebufferInfo);
    }
}