#pragma once
#ifndef VPSK_LIGHT_BUFFERS_HPP
#define VPSK_LIGHT_BUFFERS_HPP
#include "ForwardDecl.hpp"
#include <memory>
#include <vulkan/vulkan.h>

namespace vpsk {

    class LightBuffers {
        LightBuffers(const LightBuffers&) = delete;
        LightBuffers& operator=(const LightBuffers&) = delete;
    public:

        LightBuffers(const vpr::Device* dvc);
        LightBuffers(LightBuffers&& other) noexcept;
        LightBuffers& operator=(LightBuffers&& other) noexcept;
        ~LightBuffers();

        void CreateBuffers(const VkDeviceSize max_grid_count, const VkDeviceSize max_lights);
        void ClearBuffers(const VkCommandBuffer& cmd);
        void Destroy();

        std::unique_ptr<vpr::Buffer> Flags;
        std::unique_ptr<vpr::Buffer> Bounds;
        std::unique_ptr<vpr::Buffer> LightCounts;
        std::unique_ptr<vpr::Buffer> LightCountTotal;
        std::unique_ptr<vpr::Buffer> LightCountOffsets;
        std::unique_ptr<vpr::Buffer> LightList;
        std::unique_ptr<vpr::Buffer> LightCountsCompare;
    private:
        const vpr::Device* device;
    };

}

#endif //!VPSK_LIGHT_BUFFERS_HPP