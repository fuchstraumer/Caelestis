#include "objects/RenderTarget.hpp"
#include "objects/DepthTarget.hpp"
#include "RendererCore.hpp"
#include "core/LogicalDevice.hpp"
#include "resource/Image.hpp"

namespace vpsk {

    

    RenderTarget::RenderTarget() : depth(nullptr), numViews(0) {}

    RenderTarget::~RenderTarget() {
        Clear();
    }

    void RenderTarget::Create(uint32_t width, uint32_t height, const VkFormat image_format, const bool has_depth, const uint32_t mip_maps, const VkSampleCountFlagBits sample_count, bool depth_only) {
        Clear();
        auto& renderer = RendererCore::GetRenderer();
        if (!depth_only) {
            VkImageCreateInfo image_info = vpr::vk_image_create_info_base;
            image_info.extent.width = width;
            image_info.extent.height = height;
            image_info.mipLevels = mip_maps;
            image_info.samples = sample_count;
            image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            image_info.format = image_format;
            image_info.tiling = renderer.Device()->GetFormatTiling(image_format, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
            renderTargets.emplace_back(std::make_unique<vpr::Image>(renderer.Device()));
            renderTargets.back()->Create(image_info);
            renderTargets.back()->CreateView(VK_IMAGE_ASPECT_COLOR_BIT);
            if (sample_count > 1) {
                image_info.samples = VK_SAMPLE_COUNT_1_BIT;
                renderTargetsMSAA.emplace_back(std::make_unique<vpr::Image>(renderer.Device()));
                renderTargetsMSAA.back()->Create(image_info);
                renderTargetsMSAA.back()->CreateView(VK_IMAGE_ASPECT_COLOR_BIT);
                msaaUpToDate.emplace_back(VK_FALSE);
            }
            else {
                msaaUpToDate.emplace_back(VK_TRUE);
            }
        }

        Viewport.width = static_cast<float>(width);
        Viewport.height = static_cast<float>(height);
        Viewport.minDepth = 0.0f;
        Viewport.maxDepth = 1.0f;
        Viewport.x = 0;
        Viewport.y = 0;
        numViews = 1;

        if (has_depth) {
            depth = std::make_unique<DepthTarget>();
            depth->Create(width, height, sample_count);
        }

    }

    void RenderTarget::CreateAsCube(uint32_t size, const VkFormat image_format, const bool has_depth, const uint32_t mip_maps, bool depth_only) {
        Clear();
        auto& renderer = RendererCore::GetRenderer();
        if (!depth_only) {
            VkImageCreateInfo image_info = vpr::vk_image_create_info_base;
            image_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            image_info.arrayLayers = 6;
            image_info.mipLevels = mip_maps;
            image_info.extent.width = size;
            image_info.extent.height = size;
            image_info.format = image_format;
            image_info.tiling = renderer.Device()->GetFormatTiling(image_format, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
            renderTargets.emplace_back(std::make_unique<vpr::Image>(renderer.Device()));
            renderTargets.back()->Create(image_info);
            renderTargets.back()->CreateView(VK_IMAGE_ASPECT_COLOR_BIT);
            msaaUpToDate.emplace_back(VK_TRUE);
        }

        Viewport.width = static_cast<float>(size);
        Viewport.height = static_cast<float>(size);
        Viewport.minDepth = 0.0f;
        Viewport.maxDepth = 1.0f;
        Viewport.x = 0;
        Viewport.y = 0;
        numViews = 1;

        if (has_depth) {
            depth = std::make_unique<DepthTarget>();
            depth->CreateAsCube(size);
        }
    }

    void RenderTarget::Add(const VkFormat new_format) {
        
        if (renderTargets.empty()) {
            throw std::runtime_error("Tried to add format to empty rendertarget object!");
        }

        ++numViews;
        auto& renderer = RendererCore::GetRenderer();
        VkImageCreateInfo image_info = renderTargets.back()->CreateInfo();
        image_info.format = new_format;
        image_info.tiling = renderer.Device()->GetFormatTiling(new_format, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
        renderTargets.emplace_back(std::make_unique<vpr::Image>(renderer.Device()));
        renderTargets.back()->Create(image_info);
        renderTargets.back()->CreateView(VK_IMAGE_ASPECT_COLOR_BIT);
        if (image_info.samples > 1) {
            renderTargetsMSAA.emplace_back(std::make_unique<vpr::Image>(renderer.Device()));
            image_info.samples = VK_SAMPLE_COUNT_1_BIT;
            renderTargetsMSAA.back()->Create(image_info);
            renderTargetsMSAA.back()->CreateView(VK_IMAGE_ASPECT_COLOR_BIT);
            msaaUpToDate.emplace_back(VK_FALSE);
        }
        else {
            msaaUpToDate.emplace_back(VK_TRUE);
        }
    }

    void RenderTarget::Clear() {
        depth.reset();
        renderTargets.clear(); renderTargets.shrink_to_fit();
        renderTargetsMSAA.clear(); renderTargetsMSAA.shrink_to_fit();
        msaaUpToDate.clear(); msaaUpToDate.shrink_to_fit();
    }

    const vpr::Image * RenderTarget::GetImage(const size_t view_idx) const {
        return renderTargets[view_idx].get();
    }

    vpr::Image * RenderTarget::GetImage(const size_t view_idx) {
        return renderTargets[view_idx].get();
    }

    const vpr::Image * RenderTarget::GetImageMSAA(const size_t view_idx) const {
        return renderTargetsMSAA[view_idx].get();
    }

    vpr::Image * RenderTarget::GetImageMSAA(const size_t view_idx) {
        return renderTargetsMSAA[view_idx].get();
    }

}
