#pragma once
#ifndef VPSK_SHADER_RESOURCE_PACK_HPP
#define VPSK_SHADER_RESOURCE_PACK_HPP
#include "ForwardDecl.hpp"
#include <unordered_map>
#include <vector>
#include <memory>
namespace st {
    class ShaderPack;
    class ShaderResource;
    class ResourceUsage;
}

namespace vpsk {

    class RenderGraph;

    class ShaderResourcePack {
        ShaderResourcePack(const ShaderResourcePack&) = delete;
        ShaderResourcePack& operator=(const ShaderResourcePack&) = delete;
    public:

        // ShaderPacks aren't owned/loaded by this object: they are cached/stored elsewhere
        // TODO is how to load/manage these, rn thinking rendergraph should own them
        ShaderResourcePack(RenderGraph& _graph, const st::ShaderPack* pack);
        ~ShaderResourcePack();

        vpr::DescriptorSet* DescriptorSet(const char* group_name) noexcept;
        const vpr::DescriptorSet* DescriptorSet(const char* group_name) const noexcept;
        vpr::DescriptorPool* DescriptorPool() noexcept;
        const vpr::DescriptorPool* DescriptorPool() const noexcept;


    private:
        void getGroupNames();
        void createDescriptorPool();
        void createSingleSet(const std::string& name);
        void createSets();
        RenderGraph& graph;
        std::unique_ptr<vpr::DescriptorPool> descriptorPool;
        std::unordered_map<std::string, size_t> groupToIdxMap;
        std::vector<std::unique_ptr<vpr::DescriptorSet>> descriptorSets;
        const st::ShaderPack* shaderPack;
    };

}

#endif //!VPSK_SHADER_RESOURCE_PACK_HPP
