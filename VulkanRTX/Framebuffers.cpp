#include "Framebuffers.h"

#include "Device.h"
#include "Pipeline.h"
#include "Swapchain.h"

Framebuffers::Framebuffers(vk::Device& device, Swapchain& swapchain, Pipeline& pipeline) :
    device(device) {
    std::vector<vk::ImageView> swapchainImageViews = swapchain.getImageViews();

    framebuffers.resize(swapchainImageViews.size());

    for (size_t i = 0; i < swapchainImageViews.size(); ++i) {
        vk::ImageView attachments[] = {
            swapchainImageViews[i]
        };

        vk::FramebufferCreateInfo framebufferInfo;
        framebufferInfo.renderPass = pipeline.getRenderPass().getRenderPass();
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchain.getExtent().width;
        framebufferInfo.height = swapchain.getExtent().height;
        framebufferInfo.layers = 1;

        framebuffers[i] = device.createFramebuffer(framebufferInfo);
    }
}

Framebuffers::~Framebuffers() {
    for (auto framebuffer : framebuffers) {
        device.destroyFramebuffer(framebuffer);
    }
}