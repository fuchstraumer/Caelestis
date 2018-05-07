#pragma once
#ifndef VPSK_VERTEX_BUFFER_SYSTEM_HPP
#define VPSK_VERTEX_BUFFER_SYSTEM_HPP
#include "ForwardDecl.hpp"
#include <memory>
#include <unordered_map>

namespace vpsk {
    class vpskBuffer;

    class VkBufferSystem {
        VkBufferSystem(const VkBufferSystem&) = delete;
        VkBufferSystem& operator=(const VkBufferSystem&) = delete;
    public:

        enum class BufferLocation {
            Host,
            Device
        };

        VkBufferSystem(const vpr::Device* dvc);
        vpr::Buffer* AddBuffer(void* data, std::size_t data_size, const BufferLocation loc);
        vpr::Buffer* GetBuffer(const std::size_t handle);

    private:

        std::unordered_map<std::size_t, std::unique_ptr<vpskBuffer>> buffers;

    };

}

#endif //!VPSK_VERTEX_BUFFER_SYSTEM_HPP
