#pragma once
#ifndef VPSK_VERTEX_BUFFER_HPP
#define VPSK_VERTEX_BUFFER_HPP
#include "ForwardDecl.hpp"
#include <vulkan/vulkan.h>
#include <vector>

namespace vpsk {

    struct VertexBufferComponent {
        VertexBufferComponent(const std::vector<VkBuffer>& buffers) : FirstIdx(0), BindingCount(static_cast<uint32_t>(buffers.size())),
            Buffers(buffers), Offsets(size_t(buffers.size()), VkDeviceSize(0)) {}
        VertexBufferComponent(const std::vector<VkBuffer>& buffers, const std::vector<VkDeviceSize>& offsets, uint32_t idx = 0) : FirstIdx(std::move(idx)),
            Buffers(buffers), Offsets(offsets), BindingCount(static_cast<uint32_t>(buffers.size())) {}
        void operator()(VkCommandBuffer cmd) {
            vkCmdBindVertexBuffers(cmd, FirstIdx, BindingCount, Buffers.data(), Offsets.data());
        }
        uint32_t FirstIdx;
        uint32_t BindingCount;
        std::vector<VkBuffer> Buffers;
        std::vector<VkDeviceSize> Offsets;
    };

    struct IndexBufferComponent {
        IndexBufferComponent(VkBuffer handle, VkDeviceSize offset = 0, VkIndexType index_type = VK_INDEX_TYPE_UINT32) : Buffer(std::move(handle)),
            Offset(std::move(offset)), IndexType(std::move(index_type)) {}
        void operator()(VkCommandBuffer cmd) {
            vkCmdBindIndexBuffer(cmd, Buffer, Offset, IndexType);
        }
        VkBuffer Buffer{ VK_NULL_HANDLE };
        VkDeviceSize Offset{ 0 };
        VkIndexType IndexType{ VK_INDEX_TYPE_UINT32 };
    };

}

#endif // !VPSK_VERTEX_BUFFER_HPP
