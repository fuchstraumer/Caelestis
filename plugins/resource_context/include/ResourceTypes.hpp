#pragma once
#ifndef RESOURCE_CONTEXT_RESOURCE_TYPES_HPP
#define RESOURCE_CONTEXT_RESOURCE_TYPES_HPP
#include <cstdint>
#include <vulkan/vulkan.h>

enum class resource_type : uint32_t {
    INVALID = 0,
    BUFFER = 1,
    IMAGE = 2,
    SAMPLER = 3
};

enum class memory_type : uint32_t {
    INVALID_MEMORY_TYPE = 0,
    HOST_VISIBLE = 1,
    HOST_VISIBLE_AND_COHERENT = 2,
    DEVICE_LOCAL = 3,
    SPARSE = 4
};

struct gpu_resource_data_t {
    const void* Data;
    size_t DataSize;
    size_t DataAlignment;
    uint32_t Pitch;
    uint32_t SlicePitch;
};

struct VulkanResource {
    resource_type Type{ resource_type::INVALID };
    uint64_t Handle{ 0 };
    void* Info{ nullptr };
    uint64_t ViewHandle{ 0 };
    void* ViewInfo{ nullptr };
    const char* Name{ nullptr };
    void* UserData;
};

#endif //!RESOURCE_CONTEXT_RESOURCE_TYPES_HPP
