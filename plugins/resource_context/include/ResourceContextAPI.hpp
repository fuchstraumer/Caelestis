#pragma once
#ifndef RESOURCE_CONTEXT_PLUGIN_API_HPP
#define RESOURCE_CONTEXT_PLUGIN_API_HPP
#include <cstdint>

constexpr static uint32_t RESOURCE_CONTEXT_API_ID = 0x284b106d;

struct VulkanResource;
struct gpu_resource_data_t;

struct ResourceContext_API {
    VulkanResource* (*CreateBuffer)(const struct VkBufferCreateInfo* info, const struct VkBufferViewCreateInfo* view_info, const gpu_resource_data_t* initial_data, const uint32_t _memory_type, void* user_data);
    VulkanResource* (*CreateNamedBuffer)(const char* name, const struct VkBufferCreateInfo* info, const struct VkBufferViewCreateInfo* view_info, const gpu_resource_data_t* initial_data, const uint32_t _memory_type, void* user_data);
    void (*SetBufferData)(VulkanResource* dest_buffer, const gpu_resource_data_t* data);
    VulkanResource* (*CreateImage)(const struct VkImageCreateInfo* info, const struct VkImageViewCreateInfo* view_info, const gpu_resource_data_t* initial_data, const uint32_t _memory_type, void* user_data);
    VulkanResource* (*CreateNamedImage)(const char* name, const struct VkImageCreateInfo* info, const struct VkImageViewCreateInfo* view_info, const gpu_resource_data_t* initial_data, const uint32_t _memory_type, void* user_data);
    void (*SetImageData)(VulkanResource* image, const gpu_resource_data_t* data);
    VulkanResource* (*CreateSampler)(const struct VkSamplerCreateInfo* info, void* user_data);
    // Make sure dst has been initialized first.
    void (*CopyResource)(VulkanResource* src, VulkanResource* dst);
    // Unlike previous, creates a copy
    VulkanResource* (*CreateResourceCopy)(VulkanResource* src);
    void (*DestroyResource)(VulkanResource* resource);
    void (*CompletePendingTransfers)(void);
    void (*FlushStagingBuffers)(void);
    // Resource loader API
    using factory_function_t = void*(*)(const char*);
    using signal_function_t = void(*)(void*);
    void (*RegisterFileTypeFactory)(const char* file_type, factory_function_t fn);
    void (*LoadFile)(const char* file_type, const char* file_name, signal_function_t signal_fn);
    void (*UnloadFile)(const char* file_type, const char* fname);
};

#endif //!RESOURCE_CONTEXT_PLUGIN_API_HPP
