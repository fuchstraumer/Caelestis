#include "vpr_stdafx.h"
#include "render/Multisampling.hpp"
#include "render/Swapchain.hpp"
#include "resource/Image.hpp"
#include "core/LogicalDevice.hpp"

namespace vpr {
    Multisampling::Multisampling(const Device * dvc, const Swapchain * swapchain, const VkSampleCountFlagBits & sample_count) : device(dvc), 
    sampleCount(sample_count), ColorBufferMS(std::make_unique<Image>(dvc)), DepthBufferMS(std::make_unique<Image>(dvc)) {
        createColorAttachment(swapchain);
        createDepthAttachment(swapchain);
    }

    Multisampling::Multisampling(Multisampling&& other) noexcept : device(std::move(other.device)), sampleCount(std::move(other.sampleCount)),
        ColorBufferMS(std::move(other.ColorBufferMS)), DepthBufferMS(std::move(other.DepthBufferMS)) {}

    Multisampling& Multisampling::operator=(Multisampling&& other) noexcept {
        device = std::move(other.device);
        sampleCount = std::move(other.sampleCount);
        ColorBufferMS = std::move(other.ColorBufferMS);
        DepthBufferMS = std::move(other.DepthBufferMS);
        return *this;
    }

    void Multisampling::createColorAttachment(const Swapchain* swapchain) {
        VkImageCreateInfo image_info = vk_image_create_info_base;
        image_info.format = swapchain->ColorFormat();
        image_info.extent = VkExtent3D{ swapchain->Extent().width, swapchain->Extent().height, 1 };
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_info.samples = sampleCount;
        image_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_info.arrayLayers = 1;
        image_info.mipLevels = 1;

        ColorBufferMS->Create(image_info);

        VkImageViewCreateInfo msaa_view_info = vk_image_view_create_info_base;
        msaa_view_info.image = ColorBufferMS->vkHandle();
        msaa_view_info.format = swapchain->ColorFormat();

        ColorBufferMS->CreateView(msaa_view_info);
    }
    
    void Multisampling::createDepthAttachment(const Swapchain* swapchain) {
        VkImageCreateInfo image_info = vk_image_create_info_base;
        image_info.format = device->FindDepthFormat();
        image_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_info.samples = sampleCount;
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_info.arrayLayers = 1;
        image_info.mipLevels = 1;
        DepthBufferMS->Create(image_info);

        VkImageViewCreateInfo msaa_view_info = vk_image_view_create_info_base;
        msaa_view_info.image = DepthBufferMS->vkHandle();
        msaa_view_info.format = device->FindDepthFormat();
        msaa_view_info.subresourceRange = VkImageSubresourceRange{ VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };
        DepthBufferMS->CreateView(msaa_view_info);
    }

    Multisampling::~Multisampling() {
        ColorBufferMS.reset();
        DepthBufferMS.reset();
    }

}