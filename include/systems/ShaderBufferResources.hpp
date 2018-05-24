#pragma once
#ifndef VPSK_PIPELINE_RESOURCES_COMPONENT_HPP
#define VPSK_PIPELINE_RESOURCES_COMPONENT_HPP
#include "ForwardDecl.hpp"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace st {
    class ShaderGroup;
    class ShaderPack;
    class ShaderResource;
}

namespace vpsk {
    /** Represents resources that won't be loaded into or read from by the host. Shader-only buffer resources, effectively.
     *  Thus they exist a bit outside of our usual loader system.
    */
    class ShaderBufferResources {
        ShaderBufferResources(const ShaderBufferResources&) = delete;
        ShaderBufferResources& operator=(const ShaderBufferResources&) = delete;
    public:

        ShaderBufferResources(const vpr::Device* dvc, const std::vector<const st::ShaderResource*>& resources);
        vpr::Buffer* at(const char* name);
        vpr::Buffer* find(const char* name);

    private:
        void createTexelBuffer(const st::ShaderResource * texel_buffer, bool storage);
        void createUniformBuffer(const st::ShaderResource * uniform_buffer);
        void createStorageBuffer(const st::ShaderResource * storage_buffer);
        void createResources(const std::vector<const st::ShaderResource*>& resources);
        const vpr::Device* device;
        std::unordered_map<std::string, std::unique_ptr<vpr::Buffer>> buffers;
    };

}

#endif //!VPSK_PIPELINE_RESOURCE_COMPONENT_HPP