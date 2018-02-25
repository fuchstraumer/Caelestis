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

        void CreateBuffers(const uint32_t max_grid_count, const uint32_t max_lights);
        void CreateDescriptors(vpr::DescriptorPool* pool);
        void ClearBuffers(const VkCommandBuffer& cmd);
        void Destroy();

        const VkDescriptorSet& GetDescriptor() const noexcept;
        const VkDescriptorSetLayout& GetLayout() const noexcept;

        std::unique_ptr<vpr::Buffer> Flags;
        std::unique_ptr<vpr::Buffer> Bounds;
        std::unique_ptr<vpr::Buffer> LightCounts;
        std::unique_ptr<vpr::Buffer> LightCountTotal;
        std::unique_ptr<vpr::Buffer> LightCountOffsets;
        std::unique_ptr<vpr::Buffer> LightList;
        std::unique_ptr<vpr::Buffer> LightCountsCompare;
    private:

        void createSetLayout();
        void createDescriptorSet(vpr::DescriptorPool* pool);

        std::unique_ptr<vpr::DescriptorSetLayout> texelBuffersLayout;
        std::unique_ptr<vpr::DescriptorSet> texelBufferSet;
        const vpr::Device* device;
    };

}

#endif //!VPSK_LIGHT_BUFFERS_HPP