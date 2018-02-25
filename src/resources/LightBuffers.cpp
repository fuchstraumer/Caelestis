#include "resources/hpp"
#include "resource/Buffer.hpp"

namespace vpsk {

    LightBuffers::LightBuffers(const vpr::Device* dvc) : device(dvc) {}

    LightBuffers::~LightBuffers() {
        Destroy();
    }

    LightBuffers::LightBuffers(LightBuffers&& other) noexcept : Flags(std::move(other.Flags)), Bounds(std::move(other.Bounds)),
        LightCounts(std::move(other.LightCounts)), LightCountTotal(std::move(other.LightCountTotal)),
        LightCountOffsets(std::move(other.LightCountOffsets)), LightList(std::move(other.LightList)),
        LightCountsCompare(std::move(other.LightCountsCompare)), device(std::move(other.device)) {}

    LightBuffers& LightBuffers::operator=(LightBuffers&& other) noexcept {
        Flags = std::move(other.Flags);
        Bounds = std::move(other.Bounds);
        LightCounts = std::move(other.LightCounts);
        LightCountTotal = std::move(other.LightCountTotal);
        LightCountOffsets = std::move(other.LightCountOffsets);
        LightList = std::move(other.LightList);
        LightCountsCompare = std::move(other.LightCountsCompare);
        device = std::move(other.device);
        return *this;
    }

    void LightBuffers::CreateBuffers(const VkDeviceSize max_grid_count, const VkDeviceSize max_lights) {
        constexpr static auto mem_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        uint32_t indices[2]{ 0, 0 };
        VkBufferCreateInfo buffer_info = vk_buffer_create_info_base;

        if (device->QueueFamilyIndices.Graphics != device->QueueFamilyIndices.Compute) {
            buffer_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
            indices[0] = device->QueueFamilyIndices.Graphics;
            indices[1] = device->QueueFamilyIndices.Compute;
            buffer_info.pQueueFamilyIndices = indices;
            buffer_info.queueFamilyIndexCount = 2;
        }
        else {
            buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            buffer_info.pQueueFamilyIndices = nullptr;
            buffer_info.queueFamilyIndexCount = 0;
        }

        buffer_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        Flags = std::make_unique<Buffer>(device.get());
        buffer_info.size = max_grid_count * sizeof(uint8_t);
        Flags->CreateBuffer(buffer_info, mem_flags);
        Flags->CreateView(VK_FORMAT_R8_UINT, Flags->Size(), 0);

        Bounds = std::make_unique<Buffer>(device.get());
        buffer_info.size = ProgramState.MaxLights * 6 * sizeof(uint32_t);
        Bounds->CreateBuffer(buffer_info, mem_flags);
        Bounds->CreateView(VK_FORMAT_R32_UINT, Bounds->Size(), 0);

        LightCounts = std::make_unique<Buffer>(device.get());
        buffer_info.size = max_grid_count * sizeof(uint32_t);
        LightCounts->CreateBuffer(buffer_info, mem_flags);
        LightCounts->CreateView(VK_FORMAT_R32_UINT, LightCounts->Size(), 0);

        LightCountTotal = std::make_unique<Buffer>(device.get());
        buffer_info.size = sizeof(uint32_t);
        LightCountTotal->CreateBuffer(buffer_info, mem_flags);
        LightCountTotal->CreateView(VK_FORMAT_R32_UINT, LightCountTotal->Size(), 0);

        LightCountOffsets = std::make_unique<Buffer>(device.get());
        buffer_info.size = max_grid_count * sizeof(uint32_t);
        LightCountOffsets->CreateBuffer(buffer_info, mem_flags);
        LightCountOffsets->CreateView(VK_FORMAT_R32_UINT, LightCountOffsets->Size(), 0);

        LightList = std::make_unique<Buffer>(device.get());
        buffer_info.size = 1024 * 1024 * sizeof(uint32_t);
        LightList->CreateBuffer(buffer_info, mem_flags);
        LightList->CreateView(VK_FORMAT_R32_UINT, LightList->Size(), 0);

        LightCountsCompare = std::make_unique<Buffer>(device.get());
        buffer_info.size = max_grid_count * sizeof(uint32_t);
        LightCountsCompare->CreateBuffer(buffer_info, mem_flags);
        LightCountsCompare->CreateView(VK_FORMAT_R32_UINT, LightCountsCompare->Size(), 0);
    }

    void LightBuffers::ClearBuffers(const VkCommandBuffer& cmd) {
        std::vector<VkBufferMemoryBarrier> transfer_barriers{ 4, VkBufferMemoryBarrier{
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER, nullptr,
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
            VK_NULL_HANDLE, 0, 0
        } };

        transfer_barriers[0].buffer = Flags->vkHandle(); transfer_barriers[0].size = Flags->Size();
        transfer_barriers[1].buffer = LightCounts->vkHandle(); transfer_barriers[1].size = LightCounts->Size();
        transfer_barriers[2].buffer = LightCountOffsets->vkHandle(); transfer_barriers[2].size = LightCountOffsets->Size();
        transfer_barriers[3].buffer = LightList->vkHandle(); transfer_barriers[3].size = LightList->Size();

        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 4, transfer_barriers.data(), 0, nullptr);
            vkCmdFillBuffer(cmd, Flags->vkHandle(), 0, Flags->Size(), 0);
            vkCmdFillBuffer(cmd, Bounds->vkHandle(), 0, Bounds->Size(), 0);
            vkCmdFillBuffer(cmd, LightCounts->vkHandle(), 0, LightCounts->Size(), 0);
            vkCmdFillBuffer(cmd, LightCountOffsets->vkHandle(), 0, LightCountOffsets->Size(), 0);
            vkCmdFillBuffer(cmd, LightCountTotal->vkHandle(), 0, LightCountTotal->Size(), 0);
            vkCmdFillBuffer(cmd, LightList->vkHandle(), 0, LightList->Size(), 0);
            vkCmdFillBuffer(cmd, LightCountsCompare->vkHandle(), 0, LightCountsCompare->Size(), 0);
            for (auto& barrier : transfer_barriers) {
                // just invert the access masks before submitting another barrier.
                std::swap(barrier.srcAccessMask, barrier.dstAccessMask);
            }
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 4, transfer_barriers.data(), 0, nullptr);
        
    }

    void LightBuffers::Destroy() {
        Flags.reset();
        Bounds.reset();
        LightCounts.reset();
        LightCountTotal.reset();
        LightCountOffsets.reset();
        LightList.reset();
        LightCountsCompare.reset();
    }
}