#pragma once
#ifndef RESOURCE_CONTEXT_PLUGIN_API_HPP
#define RESOURCE_CONTEXT_PLUGIN_API_HPP
#include <cstdint>

constexpr static uint32_t RESOURCE_CONTEXT_API_ID = 0x284b106d;

struct VulkanResource;
struct gpu_resource_data_t;

struct ResourceContext_API {
    VulkanResource* (*CreateBuffer, )(const struct VkBufferCreateInfo* info, const struct VkBufferViewCreateInfo* view_info, const gpu_resource_data_t* initial_data);
    void (*SetBufferData)(VulkanResource* dest_buffer, const gpu_resource_data_t* data);
    VulkanResource* (*CreateImage)(const struct VkImageCreateInfo* info, const struct VkImageViewCreateInfo* view_info, const gpu_resource_data_t* initial_data);
    void (*SetImageData)(VulkanResource* image, const gpu_resource_data_t* data);
    VulkanResource* (*CreateSampler)(const struct VkSamplerCreateInfo* info);
    void (*DestroyResource)(VulkanResource* resource);
};

#endif //!RESOURCE_CONTEXT_PLUGIN_API_HPP
