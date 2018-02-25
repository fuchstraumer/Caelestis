#include "render/FrameData.hpp"
#include "core/LogicalDevice.hpp"

namespace vpsk {

    FrameData::FrameData(const vpr::Device* dvc) : device(dvc) {}

    FrameData::FrameData(FrameData && other) noexcept : device(std::move(other.device)), cmdBlocks(std::move(other.cmdBlocks)) {
        other.cmdBlocks.clear();
    }

    FrameData & FrameData::operator=(FrameData && other) noexcept{
        device = std::move(other.device);
        cmdBlocks = std::move(other.cmdBlocks);
        other.cmdBlocks.clear();
        return *this;
    }

    FrameData::~FrameData() {
        for (auto& block : cmdBlocks) {
            if (block.second.Fence != VK_NULL_HANDLE) {
                vkDestroyFence(device->vkHandle(), block.second.Fence, nullptr);
            }
        }
    }

    FrameData & FrameData::AddCommandBlock(const std::string & name, VkCommandBuffer cmd) {
        VkFence fence = VK_NULL_HANDLE;
        vkCreateFence(device->vkHandle(), &vpr::vk_fence_create_info_base, nullptr, &fence);
        cmdBlocks.emplace(name, cmd_buffer_block_t{ cmd, fence, true });
    }

    cmd_buffer_block_t & FrameData::GetCommandBlock(const std::string & name) {
        return cmdBlocks.at(name);
    }

    

}