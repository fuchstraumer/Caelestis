#pragma once
#ifndef RESOURCE_CONTEXT_HPP
#define RESOURCE_CONTEXT_HPP
#include "vpr/ForwardDecl.hpp"
#include "vpr/Allocation.hpp"
#include "vpr/AllocationRequirements.hpp"
#include "ResourceTypes.hpp"
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <string>
#include <variant>
#include <vulkan/vulkan.h>

class ResourceContext {
    ResourceContext(const ResourceContext&) = delete;
    ResourceContext& operator=(const ResourceContext&) = delete;
public:
    // Resource context is bound to a single device and uses underlying physical device resources
    ResourceContext(vpr::Device* device, vpr::PhysicalDevice* physical_device);
    ~ResourceContext();

    VulkanResource* CreateBuffer(const VkBufferCreateInfo* info, const VkBufferViewCreateInfo* view_info, const gpu_resource_data_t* initial_data, const memory_type _memory_type, void* user_data = nullptr);
    VulkanResource* CreateNamedBuffer(const char* name, const VkBufferCreateInfo* info, const VkBufferViewCreateInfo* view_info, const gpu_resource_data_t* initial_data, const memory_type _memory_type, void* user_data = nullptr);
    void SetBufferData(VulkanResource* dest_buffer, const gpu_resource_data_t* data);
    VulkanResource* CreateImage(const VkImageCreateInfo* info, const VkImageViewCreateInfo* view_info, const gpu_resource_data_t* initial_data, const memory_type _memory_type, void* user_data = nullptr);
    void SetImageData(VulkanResource* image, const gpu_resource_data_t* data);
    VulkanResource* CreateSampler(const VkSamplerCreateInfo* info, void* user_data = nullptr);
    void DestroyResource(VulkanResource* resource);

    void* MapResourceMemory(VulkanResource* resource, size_t size = 0, size_t offset = 0);
    void UnmapResourceMemory(VulkanResource* resource);

    // Call at start of frame
    void Update();
    // Call at end of frame
    void FlushStagingBuffers();

private:

    void setBufferInitialDataHostOnly(VulkanResource * resource, const gpu_resource_data_t * initial_data, vpr::Allocation& alloc, memory_type _memory_type);
    void setBufferInitialDataUploadBuffer(VulkanResource* resource, const gpu_resource_data_t* initial_data, vpr::Allocation& alloc);
    vpr::AllocationRequirements getAllocReqs(memory_type _memory_type) const noexcept;
    VkFormatFeatureFlags featureFlagsFromUsage(const VkImageUsageFlags flags) const noexcept;

    void destroyBuffer(VulkanResource* rsrc);
    void destroyImage(VulkanResource* rsrc);
    void destroySampler(VulkanResource* rsrc);

    struct infoStorage {
        std::unordered_map<VulkanResource*, memory_type> resourceMemoryType;
        std::unordered_map<VulkanResource*, VkBufferCreateInfo> bufferInfos;
        std::unordered_map<VulkanResource*, VkBufferViewCreateInfo> bufferViewInfos;
        std::unordered_map<VulkanResource*, VkImageCreateInfo> imageInfos;
        std::unordered_map<VulkanResource*, VkImageViewCreateInfo> imageViewInfos;
        std::unordered_map<VulkanResource*, VkSamplerCreateInfo> samplerInfos;
    } resourceInfos;

    std::unordered_map<VulkanResource*, std::string> resourceNames;
    std::unordered_map<VulkanResource*, vpr::Allocation> resourceAllocations;
    std::unordered_map<VulkanResource*, VkMappedMemoryRange> mappedRanges;
    std::unordered_set<std::unique_ptr<VulkanResource>> resources;
    std::unique_ptr<vpr::Allocator> allocator;
    const vpr::Device* device;

};

#endif //!RESOURCE_CONTEXT_HPP
