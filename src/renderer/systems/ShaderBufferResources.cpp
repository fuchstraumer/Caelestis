#include "renderer/systems/ShaderBufferResources.hpp"
#include "core/ShaderResource.hpp"
#include "resource/Buffer.hpp"

namespace vpsk {

    ShaderBufferResources::ShaderBufferResources(const vpr::Device* dvc, const std::vector<const st::ShaderResource*>& resources) : device(dvc) {
        createResources(resources);
    }

    vpr::Buffer * ShaderBufferResources::at(const char * name) {
        return buffers.at(name).get();
    }

    vpr::Buffer * ShaderBufferResources::find(const char * name) {
        auto iter = buffers.find(name);
        if (iter != buffers.cend()) {
            return iter->second.get();
        }
        else {
            return nullptr;
        }
    }

    void ShaderBufferResources::createTexelBuffer(const st::ShaderResource* texel_buffer, bool storage) {
        auto buffer = std::make_unique<vpr::Buffer>(device);
        auto flags = storage ? VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT : VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
        buffer->CreateBuffer(flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texel_buffer->GetAmountOfMemoryRequired());
        buffer->CreateView(texel_buffer->GetFormat(), buffer->Size(), 0);
        buffers.emplace(texel_buffer->GetName(), std::move(buffer));
    }

    void ShaderBufferResources::createUniformBuffer(const st::ShaderResource* uniform_buffer) {
        auto buffer = std::make_unique<vpr::Buffer>(device);
        auto flags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        buffer->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, flags, uniform_buffer->GetAmountOfMemoryRequired());
        buffers.emplace(uniform_buffer->GetName(), std::move(buffer));
    }

    void ShaderBufferResources::createStorageBuffer(const st::ShaderResource* storage_buffer) {
        auto buffer = std::make_unique<vpr::Buffer>(device);
        buffer->CreateBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, storage_buffer->GetAmountOfMemoryRequired());
        buffers.emplace(storage_buffer->GetName(), std::move(buffer));
    }

    void ShaderBufferResources::createResources(const std::vector<const st::ShaderResource*>& resources) {
        for (const auto& rsrc : resources) {
            switch (rsrc->GetType()) {
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

}