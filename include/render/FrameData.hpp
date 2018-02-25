#pragma once
#ifndef VPSK_FRAME_DATA_HPP
#define VPSK_FRAME_DATA_HPP
#include "ForwardDecl.hpp"
#include <memory>
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <string>

namespace vpsk {

    struct cmd_buffer_block_t {
        VkCommandBuffer Cmd = VK_NULL_HANDLE;
        VkFence Fence = VK_NULL_HANDLE;
        bool firstFrame = true;
    };

    class FrameData {
        FrameData(const FrameData&) = delete;
        FrameData& operator=(const FrameData&) = delete;
    public:

        FrameData(const vpr::Device* dvc);
        FrameData(FrameData&& other) noexcept;
        FrameData& operator=(FrameData&& other) noexcept;
        ~FrameData();

        FrameData& AddCommandBlock(const std::string& name, VkCommandBuffer cmd);
        cmd_buffer_block_t& GetCommandBlock(const std::string& name);

    private:
        const vpr::Device* device;
        std::unordered_map<std::string, cmd_buffer_block_t> cmdBlocks;
    };

}

#endif //!VPSK_FRAME_DATA_HPP