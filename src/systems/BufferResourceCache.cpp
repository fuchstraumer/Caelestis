#include "systems/BufferResourceCache.hpp"
#include "core/ShaderResource.hpp"
#include "resource/Buffer.hpp"

namespace vpsk {

    BufferResourceCache::BufferResourceCache(const vpr::Device* dvc) : device(dvc) { }

    BufferResourceCache::~BufferResourceCache() {}

    void BufferResourceCache::AddResources(const std::vector<const st::ShaderResource*>& resources) {
        createResources(resources);
    }

    void BufferResourceCache::AddResource(const st::ShaderResource* resource) {
        if (buffers.count(resource->Name()) == 0) {
            createResource(resource);
        }
    }

    vpr::Buffer* BufferResourceCache::at(const char* name) {
        return buffers.at(name).get();
    }

    vpr::Buffer* BufferResourceCache::find(const char* name) {
        auto iter = buffers.find(name);
        if (iter != buffers.cend()) {
            return iter->second.get();
        }
        else {
            return nullptr;
        }
    }

    void BufferResourceCache::createTexelBuffer(const st::ShaderResource* texel_buffer, bool storage) {
        auto buffer = std::make_unique<vpr::Buffer>(device);
        auto flags = storage ? VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT : VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
        buffer->CreateBuffer(flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texel_buffer->MemoryRequired());
        buffer->CreateView(texel_buffer->Format(), buffer->Size(), 0);
        buffers.emplace(texel_buffer->Name(), std::move(buffer));
    }

    void BufferResourceCache::createUniformBuffer(const st::ShaderResource* uniform_buffer) {
        auto buffer = std::make_unique<vpr::Buffer>(device);
        auto flags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        buffer->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, flags, uniform_buffer->MemoryRequired());
        buffers.emplace(uniform_buffer->Name(), std::move(buffer));
    }

    void BufferResourceCache::createStorageBuffer(const st::ShaderResource* storage_buffer) {
        auto buffer = std::make_unique<vpr::Buffer>(device);
        buffer->CreateBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, storage_buffer->MemoryRequired());
        buffers.emplace(storage_buffer->Name(), std::move(buffer));
    }

    void BufferResourceCache::createResources(const std::vector<const st::ShaderResource*>& resources) {
        for (const auto& rsrc : resources) {
            if (buffers.count(rsrc->Name()) == 0) {
                createResource(rsrc);
            }
        }
    }

    void BufferResourceCache::createResource(const st::ShaderResource* rsrc) {
        switch (rsrc->DescriptorType()) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            createTexelBuffer(rsrc, false);
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            createTexelBuffer(rsrc, true);
            break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            createUniformBuffer(rsrc);
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            createStorageBuffer(rsrc);
            break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            createUniformBuffer(rsrc);
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            createStorageBuffer(rsrc);
            break;
        default:
            break;
        }
    }
}

