#include "ResourceContext.hpp"
#include "TransferSystem.hpp"
#include "vpr/Allocator.hpp"
#include "vpr/AllocationRequirements.hpp"
#include "vpr/Allocation.hpp"
#include "vpr/LogicalDevice.hpp"
#include "vpr/PhysicalDevice.hpp"
#include "vpr/vkAssert.hpp"
#include "UploadBuffer.hpp"
#include <foonathan/memory/memory_stack.hpp>
#include <vector>

struct userDataCopies {
    foonathan::memory::memory_stack<> allocator;
    std::vector<void*> storedAddresses;

    void* allocate(size_t size, size_t alignment) {
        void* result = allocator.allocate(size, alignment);
        storedAddresses.emplace_back(result);
        return result;
    }

    void flush() {
        storedAddresses.clear(); storedAddresses.shrink_to_fit();
        allocator.shrink_to_fit();
    }
};

static userDataCopies userData;
static std::vector<std::unique_ptr<UploadBuffer>> uploadBuffers;

static VkAccessFlags accessFlagsFromBufferUsage(VkBufferUsageFlags usage_flags) {
    if (usage_flags & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) {
        return VK_ACCESS_UNIFORM_READ_BIT;
    }
    else if (usage_flags & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) {
        return VK_ACCESS_INDEX_READ_BIT;
    }
    else if (usage_flags & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) {
        return VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    }
    else if (usage_flags & VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT) {
        return VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    }
#ifdef VK_API_1_8_0
    else if (usage_flags & VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT) {
        return VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT;
    }
#endif
    else if ((usage_flags & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) || (usage_flags & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)) { 
        return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    }
    else if (usage_flags & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) {
        return VK_ACCESS_SHADER_READ_BIT;
    }
    else {
        return VK_ACCESS_MEMORY_READ_BIT;
    }
}

static VkAccessFlags accessFlagsFromImageUsage(const VkImageUsageFlags usage_flags) {
    if (usage_flags & VK_IMAGE_USAGE_SAMPLED_BIT) {
        return VK_ACCESS_SHADER_READ_BIT;
    }
    else if (usage_flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
        return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }
    else if (usage_flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }
    else if (usage_flags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) {
        return VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    }
    else {
        return VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_MEMORY_READ_BIT;
    }
}


vpr::Allocator::allocation_extensions getExtensionFlags(const vpr::Device* device) {
    return device->DedicatedAllocationExtensionsEnabled() ? vpr::Allocator::allocation_extensions::DedicatedAllocations : vpr::Allocator::allocation_extensions::None;
}

ResourceContext::ResourceContext(vpr::Device * _device, vpr::PhysicalDevice * physical_device) : device(_device), allocator(std::make_unique<vpr::Allocator>(_device->vkHandle(), physical_device->vkHandle(), getExtensionFlags(_device))) {
    auto& transfer_system = ResourceTransferSystem::GetTransferSystem();
    transfer_system.Initialize(_device);
}

ResourceContext::~ResourceContext() {}

VulkanResource* ResourceContext::CreateBuffer(const VkBufferCreateInfo* info, const VkBufferViewCreateInfo* view_info, const gpu_resource_data_t* initial_data, const memory_type _memory_type, void* user_data) {

    auto iter = resources.emplace(std::make_unique<VulkanResource>());
    VulkanResource* resource = iter.first->get();

    auto info_iter = resourceInfos.bufferInfos.emplace(resource, *info);
    resource->Type = resource_type::BUFFER;
    resource->Info = &info_iter.first->second;
    resource->UserData = user_data;
    
    VkResult result = vkCreateBuffer(device->vkHandle(), info, nullptr, reinterpret_cast<VkBuffer*>(&resource->Handle));
    VkAssert(result);

    if (view_info) {
        auto view_iter = resourceInfos.bufferViewInfos.emplace(resource, *view_info);
        resource->ViewInfo = &view_iter.first->second;
        result = vkCreateBufferView(device->vkHandle(), &view_iter.first->second, nullptr, reinterpret_cast<VkBufferView*>(&resource->ViewHandle));
        VkAssert(result);
    }

    resourceInfos.resourceMemoryType.emplace(resource, _memory_type);
    auto alloc_iter = resourceAllocations.emplace(resource, vpr::Allocation());
    vpr::Allocation& alloc = alloc_iter.first->second;
    allocator->AllocateForBuffer(reinterpret_cast<VkBuffer&>(resource->Handle), getAllocReqs(_memory_type), vpr::AllocationType::Buffer, alloc);

    if (initial_data) {
        if ((_memory_type == memory_type::HOST_VISIBLE) || (_memory_type == memory_type::HOST_VISIBLE_AND_COHERENT)) {
            setBufferInitialDataHostOnly(resource, initial_data, alloc, _memory_type);
        }
        else {
            setBufferInitialDataUploadBuffer(resource, initial_data, alloc);
        }
    }

    return resource;
}

VulkanResource* ResourceContext::CreateNamedBuffer(const char* name, const VkBufferCreateInfo* info, const VkBufferViewCreateInfo* view_info, const gpu_resource_data_t* initial_data, const memory_type _memory_type, void* user_data) {
    VulkanResource* result = CreateBuffer(info, view_info, initial_data, _memory_type, user_data);
    auto iter = resourceNames.emplace(result, std::string(name));
    result->Name = iter.first->second.c_str();
    return result;
}

void ResourceContext::SetBufferData(VulkanResource* dest_buffer, const gpu_resource_data_t* data) {
    memory_type mem_type = resourceInfos.resourceMemoryType.at(dest_buffer);
    if ((mem_type == memory_type::HOST_VISIBLE) || (mem_type == memory_type::HOST_VISIBLE_AND_COHERENT)) {
        setBufferInitialDataHostOnly(dest_buffer, data, resourceAllocations.at(dest_buffer), mem_type);
    }
    else {
        setBufferInitialDataUploadBuffer(dest_buffer, data, resourceAllocations.at(dest_buffer));
    }
}

VulkanResource* ResourceContext::CreateImage(const VkImageCreateInfo* info, const VkImageViewCreateInfo* view_info, const gpu_resource_data_t* initial_data, const memory_type _memory_type, void* user_data) {
    VulkanResource* resource = nullptr;
    {
        auto iter = resources.emplace(std::make_unique<VulkanResource>());
        resource = iter.first->get();
    }

    auto info_iter = resourceInfos.imageInfos.emplace(resource, *info);
    resource->Type = resource_type::IMAGE;
    resource->Info = &info_iter.first->second;
    resource->UserData = user_data;

    // This probably isn't ideal but it's a reasonable assumption to make.
    VkImageCreateInfo* create_info = reinterpret_cast<VkImageCreateInfo*>(resource->Info);
    create_info->usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkResult result = vkCreateImage(device->vkHandle(), create_info, nullptr, reinterpret_cast<VkImage*>(&resource->Handle));
    VkAssert(result);

    if (view_info) {
        auto view_iter = resourceInfos.imageViewInfos.emplace(resource, *view_info);
        resource->ViewInfo = &view_iter.first->second;
        result = vkCreateImageView(device->vkHandle(), view_info, nullptr, reinterpret_cast<VkImageView*>(&resource->ViewHandle));
        VkAssert(result);
    }

    vpr::AllocationType alloc_type;
    VkImageTiling format_tiling = device->GetFormatTiling(info->format, featureFlagsFromUsage(info->usage));
    if (format_tiling == VK_IMAGE_TILING_LINEAR) {
        alloc_type = vpr::AllocationType::ImageLinear;
    }
    else if (format_tiling == VK_IMAGE_TILING_OPTIMAL) {
        alloc_type = vpr::AllocationType::ImageTiled;
    }
    else {
        alloc_type = vpr::AllocationType::Unknown;
    }

    resourceInfos.resourceMemoryType.emplace(resource, _memory_type);
    auto alloc_iter = resourceAllocations.emplace(resource, vpr::Allocation());
    vpr::Allocation& alloc = alloc_iter.first->second;
    allocator->AllocateForImage(reinterpret_cast<VkImage&>(resource->Handle), getAllocReqs(_memory_type), alloc_type, alloc);

    if (initial_data) {
        
    }

    return resource;
}

VulkanResource* ResourceContext::CreateSampler(const VkSamplerCreateInfo* info, void* user_data) {
    VulkanResource* resource = nullptr; 
    {
        auto iter = resources.emplace(std::make_unique<VulkanResource>());
        resource = iter.first->get();
    }

    auto info_iter = resourceInfos.samplerInfos.emplace(resource, *info);
    resource->Type = resource_type::SAMPLER;
    resource->Info = &info_iter.first->second;
    resource->UserData = user_data;

    VkResult result = vkCreateSampler(device->vkHandle(), info, nullptr, reinterpret_cast<VkSampler*>(&resource->Handle));
    VkAssert(result);

    return resource;
}

void* ResourceContext::MapResourceMemory(VulkanResource* resource, size_t size = 0, size_t offset = 0) {
    void* mapped_ptr = nullptr;
    auto& alloc = resourceAllocations.at(resource);
    if (resourceInfos.resourceMemoryType.at(resource) == memory_type::HOST_VISIBLE) {
        mappedRanges[resource] = VkMappedMemoryRange{ VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, nullptr, alloc.Memory(), alloc.Offset() + offset, size == 0 ? alloc.Size : size }; 
        VkResult result = vkInvalidateMappedMemoryRanges(device->vkHandle(), 1, &mappedRanges[resource]);
        VkAssert(result);
    }
    alloc.Map(size == 0 ? alloc.Size : size, alloc.Offset() + offset, mapped_ptr);
    return mapped_ptr;
}

void ResourceContext::UnmapResourceMemory(VulkanResource * resource) {
    resourceAllocations.at(resource).Unmap();
    if (resourceInfos.resourceMemoryType.at(resource) == memory_type::HOST_VISIBLE) {
        VkResult result = vkFlushMappedMemoryRanges(device->vkHandle(), 1, &mappedRanges[resource]);
        VkAssert(result);
    }
}

void ResourceContext::Update() {
    auto& transfer_system = ResourceTransferSystem::GetTransferSystem();
    transfer_system.CompleteTransfers();
}

void ResourceContext::FlushStagingBuffers() {
    for (auto& buff : uploadBuffers) {
        allocator->FreeMemory(&buff->alloc);
        vkDestroyBuffer(device->vkHandle(), buff->Buffer, nullptr);
        buff.reset();
    }
    uploadBuffers.clear(); uploadBuffers.shrink_to_fit();
    userData.flush();
}

void ResourceContext::setBufferInitialDataHostOnly(VulkanResource* resource, const gpu_resource_data_t* initial_data, vpr::Allocation& alloc, memory_type _memory_type) {
    void* mapped_address = nullptr;
    alloc.Map(alloc.Size, alloc.Offset(), mapped_address);
    memcpy(mapped_address, initial_data->Data, initial_data->DataSize);
    alloc.Unmap();
    if (_memory_type == memory_type::HOST_VISIBLE) {
        VkMappedMemoryRange mapped_memory{ VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, nullptr, alloc.Memory(), alloc.Offset(), alloc.Size };
        VkResult result = vkFlushMappedMemoryRanges(device->vkHandle(), 1, &mapped_memory);
        VkAssert(result);
    }
}

void ResourceContext::setBufferInitialDataUploadBuffer(VulkanResource* resource, const gpu_resource_data_t* initial_data, vpr::Allocation& alloc) {
    // first copy user data into another buffer: we don't know how long the users data will persist, and we
    // need it to last until this submission completes. so lets take care of that ourself.
    void* user_data_copy = userData.allocate(initial_data->DataSize, initial_data->DataAlignment);
    memcpy(user_data_copy, initial_data->Data, initial_data->DataSize);
    uploadBuffers.emplace_back(std::make_unique<UploadBuffer>(device, allocator, initial_data->DataSize));
    uploadBuffers.back()->SetData(user_data_copy, initial_data->DataSize);
    auto& transfer_system = ResourceTransferSystem::GetTransferSystem();
    {
        auto& guard = transfer_system.AcquireSpinLock();
        auto cmd = transfer_system.TransferCmdBuffer();
        const VkBufferMemoryBarrier memory_barrier0 {
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            nullptr,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            reinterpret_cast<VkBuffer>(resource->Handle),
            0,
            alloc.Size
        };
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, 0, 1, &memory_barrier0, 0, nullptr);
        const VkBufferCopy copy_region{
            0,
            0,
            alloc.Size
        };
        vkCmdCopyBuffer(cmd, uploadBuffers.back()->Buffer, reinterpret_cast<VkBuffer>(resource->Handle), 1, &copy_region);
        const VkBufferCreateInfo* p_info = reinterpret_cast<VkBufferCreateInfo*>(resource->Info);
        const VkBufferMemoryBarrier memory_barrier1 {
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            nullptr,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            accessFlagsFromBufferUsage(p_info->usage),
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            0,
            alloc.Size
        };
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 1, &memory_barrier1, 0, nullptr);
    }
}

vpr::AllocationRequirements ResourceContext::getAllocReqs(memory_type _memory_type) const noexcept {
    vpr::AllocationRequirements alloc_reqs;
    switch (_memory_type) {
    case memory_type::HOST_VISIBLE:
        alloc_reqs.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        break;
    case memory_type::HOST_VISIBLE_AND_COHERENT:
        alloc_reqs.requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        break;
    case memory_type::DEVICE_LOCAL:
        alloc_reqs.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        break;
    case memory_type::SPARSE:
        break;
    default:
        break;
    }
    return alloc_reqs;
}

VkFormatFeatureFlags ResourceContext::featureFlagsFromUsage(const VkImageUsageFlags flags) const noexcept {
    VkFormatFeatureFlags result = 0;
    if (flags & VK_IMAGE_USAGE_SAMPLED_BIT) {
        result |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
    }
    if (flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
        result |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
    }
    if (flags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        result |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if (flags & VK_IMAGE_USAGE_STORAGE_BIT) {
        result |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
    }
    return result;
}
