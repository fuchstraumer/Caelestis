#pragma once
#ifndef RESOURCE_CONTEXT_UPLOAD_BUFFER_HPP
#define RESOURCE_CONTEXT_UPLOAD_BUFFER_HPP
#include "vpr/Allocator.hpp"
#include "vpr/LogicalDevice.hpp"
#include "vpr/vkAssert.hpp"
#include "vpr/AllocationRequirements.hpp"
#include "vpr/Allocation.hpp"
#include <vulkan/vulkan.h>

constexpr static VkBufferCreateInfo staging_buffer_create_info{
    VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    nullptr,
    0,
    0,
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_SHARING_MODE_EXCLUSIVE,
    0,
    nullptr
};

struct UploadBuffer {
    UploadBuffer(const vpr::Device* _device, vpr::Allocator* alloc, VkDeviceSize sz);
    void SetData(const void* data, size_t data_size);
    VkBuffer Buffer;
    vpr::Allocation alloc;
    const vpr::Device* device;
};

UploadBuffer::UploadBuffer(const vpr::Device * _device, vpr::Allocator * allocator, VkDeviceSize sz) : device(_device) {
    VkBufferCreateInfo create_info = staging_buffer_create_info;
    create_info.size = sz;
    VkResult result = vkCreateBuffer(device->vkHandle(), &create_info, nullptr, &Buffer);
    VkAssert(result);
    vpr::AllocationRequirements alloc_reqs;
    alloc_reqs.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    allocator->AllocateForBuffer(Buffer, alloc_reqs, vpr::AllocationType::Buffer, alloc);
}

void UploadBuffer::SetData(const void* data, size_t data_size) {
    VkMappedMemoryRange mapped_memory{ VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, nullptr, alloc.Memory(), alloc.Offset(), alloc.Size };
    void* mapped_address = nullptr;
    alloc.Map(alloc.Size, alloc.Offset, mapped_address);
    memcpy(mapped_address, data, data_size);
    alloc.Unmap();
    VkResult result = vkFlushMappedMemoryRanges(device->vkHandle(), 1, &mapped_memory);
    VkAssert(result);
}

#endif //!RESOURCE_CONTEXT_UPLOAD_BUFFER_HPP
