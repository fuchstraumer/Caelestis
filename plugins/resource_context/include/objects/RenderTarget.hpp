#pragma once
#ifndef VPSK_RENDER_TARGET_HPP
#define VPSK_RENDER_TARGET_HPP
#include "ForwardDecl.hpp"
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
namespace vpsk {

    class DepthTarget;

    class RenderTarget {
        RenderTarget(const RenderTarget& other) = delete;
        RenderTarget& operator=(const RenderTarget& other) = delete;
    public:

        RenderTarget();
        ~RenderTarget();

        void Create(uint32_t width, uint32_t height, const VkFormat image_format, const bool has_depth = false, const uint32_t mip_maps = 1,
            const VkSampleCountFlags sample_count = VK_SAMPLE_COUNT_1_BIT, bool depth_only = false);
        void CreateAsCube(uint32_t size, const VkFormat image_format, const bool has_depth = false, const uint32_t mip_maps = 1, bool depth_only = false);
        void Add(const VkFormat new_format);

        void Clear();

        const vpr::Image* GetImage(const size_t view_idx = 0) const;
        vpr::Image* GetImage(const size_t view_idx = 0);
        const vpr::Image* GetImageMSAA(const size_t view_idx = 0) const;
        vpr::Image* GetImageMSAA(const size_t view_idx = 0);
        image_info_t GetImageInfo() const;

        VkViewport Viewport;
    private:
        std::unique_ptr<DepthTarget> depth;
        size_t numViews;
        std::vector<std::unique_ptr<vpr::Image>> renderTargets;
        std::vector<std::unique_ptr<vpr::Image>> renderTargetsMSAA;
        std::vector<uint32_t> msaaUpToDate;
    };

}

#endif // !VPSK_RENDER_TARGET_HPP
