#pragma once
#ifndef RENDERER_CONTEXT_TESTS_COMMON_CREATION_FUNCTIONS_HPP
#define RENDERER_CONTEXT_TESTS_COMMON_CREATION_FUNCTIONS_HPP
#include <vulkan/vulkan.h>
#include <array>

namespace vpr {
    class Instance;
    class PhysicalDevice;
    class Device;
    class Swapchain;
}

struct DepthStencil {
    VkImage Image;
    VkDeviceMemory Memory;
    VkImageView View;
    VkFormat Format;
};

uint32_t GetMemoryTypeIndex(uint32_t type_bits, VkMemoryPropertyFlags properties, VkPhysicalDeviceMemoryProperties memory_properties);
DepthStencil CreateDepthStencil(const vpr::Device* device, const vpr::PhysicalDevice* physical_device, const vpr::Swapchain* swapchain);
VkRenderPass CreateBasicRenderpass(const vpr::Device* device, const vpr::Swapchain* swapchain, VkFormat depth_format);
VkPipeline CreateBasicPipeline(const vpr::Device* device, uint32_t num_stages, const VkPipelineShaderStageCreateInfo* pStages, const VkPipelineVertexInputStateCreateInfo* vertex_state, VkPipelineLayout pipeline_layout,
        VkRenderPass renderpass);

#endif //!RENDERER_CONTEXT_TESTS_COMMON_CREATION_FUNCTIONS_HPP