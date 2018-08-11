#pragma once
#ifndef RESOURCE_CONTEXT_PLUGIN_API_HPP
#define RESOURCE_CONTEXT_PLUGIN_API_HPP
#include <cstdint>
#include <cstddef>

constexpr static uint32_t RESOURCE_CONTEXT_API_ID = 0x284b106d;

struct VulkanResource;
struct gpu_resource_data_t;
struct gpu_image_resource_data_t;

struct ResourceContext_API {
    VulkanResource* (*CreateBuffer)(const struct VkBufferCreateInfo* info, const struct VkBufferViewCreateInfo* view_info, const size_t num_data, const gpu_resource_data_t* initial_data, const uint32_t _memory_type, void* user_data);
    VulkanResource* (*CreateNamedBuffer)(const char* name, const struct VkBufferCreateInfo* info, const struct VkBufferViewCreateInfo* view_info, const size_t num_data, const gpu_resource_data_t* initial_data, const uint32_t _memory_type, void* user_data);
    void (*SetBufferData)(VulkanResource* dest_buffer, const size_t num_data, const gpu_resource_data_t* data);
    VulkanResource* (*CreateImage)(const struct VkImageCreateInfo* info, const struct VkImageViewCreateInfo* view_info, const size_t num_data, const gpu_image_resource_data_t* initial_data, const uint32_t _memory_type, void* user_data);
    VulkanResource* (*CreateNamedImage)(const char* name, const struct VkImageCreateInfo* info, const struct VkImageViewCreateInfo* view_info, const size_t num_data, const gpu_image_resource_data_t* initial_data, const uint32_t _memory_type, void* user_data);
    void (*SetImageData)(VulkanResource* image, const size_t num_data, const gpu_image_resource_data_t* data);
    VulkanResource* (*CreateSampler)(const struct VkSamplerCreateInfo* info, void* user_data);
    // Make sure dst has been initialized first.
    void (*CopyResource)(VulkanResource* src, VulkanResource* dst);
    // Unlike previous, creates a copy
    VulkanResource* (*CreateResourceCopy)(VulkanResource* src);
    void (*DestroyResource)(VulkanResource* resource);
    void (*CompletePendingTransfers)(void);
    void (*FlushStagingBuffers)(void);
    // Resource loader API
    using factory_function_t = void*(*)(const char* fname);
    using deleter_function_t = void(*)(void* object_instance);
    using signal_function_t = void(*)(void* object_or_state, void* data);
    void (*RegisterFileTypeFactory)(const char* file_type, factory_function_t fn, deleter_function_t del_fn);
    // Load a file of a type previously registered with RegisterFileFactory(). file_name is the objects path (can be relative, will be made canonical/absolute). 
    // requesting_object_ptr is a potential state/class pointer that can be used by signal_fn, to allow for calling class/struct member functions on signal receipt.
    void (*LoadFile)(const char* file_type, const char* file_name, void* requesting_object_ptr, signal_function_t signal_fn);
    void (*UnloadFile)(const char* file_type, const char* fname);
};

#endif //!RESOURCE_CONTEXT_PLUGIN_API_HPP
