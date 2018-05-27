#include "graph/ShaderResourcePack.hpp"
#include "resource/DescriptorPool.hpp"
#include "resource/DescriptorSet.hpp"
#include "core/ShaderPack.hpp"
#include "graph/RenderGraph.hpp"
#include "systems/BufferResourceCache.hpp"
#include "RendererCore.hpp"
#include "core/ShaderResource.hpp"
#include "resource/Buffer.hpp"

namespace vpsk {

    ShaderResourcePack::ShaderResourcePack(RenderGraph& _graph, const st::ShaderPack * pack) : shaderPack(pack), graph(_graph) {
        getGroupNames();
        createDescriptorPool();
        createSets();
    }

    ShaderResourcePack::~ShaderResourcePack() {}

    vpr::DescriptorSet* ShaderResourcePack::DescriptorSet(const char* group_name) noexcept {
        auto iter = groupToIdxMap.find(group_name);
        if (iter != groupToIdxMap.end()) {
            return descriptorSets[groupToIdxMap.at(group_name)].get();
        }
        else {
            return nullptr;
        }
    }

    const vpr::DescriptorSet* ShaderResourcePack::DescriptorSet(const char* group_name) const noexcept {
        auto iter = groupToIdxMap.find(group_name);
        if (iter != groupToIdxMap.cend()) {
            return descriptorSets[groupToIdxMap.at(group_name)].get();
        }
        else {
            return nullptr;
        }
    }

    vpr::DescriptorPool* ShaderResourcePack::DescriptorPool() noexcept {
        return descriptorPool.get();
    }

    const vpr::DescriptorPool* ShaderResourcePack::DescriptorPool() const noexcept {
        return descriptorPool.get();
    }

    void ShaderResourcePack::getGroupNames() {
        auto names = shaderPack->GetResourceGroupNames();
        for (size_t i = 0; i < names.NumStrings; ++i) {
            groupToIdxMap.emplace(names.Strings[i], i);
        }
    }

    void ShaderResourcePack::createDescriptorPool() {
        auto& renderer = RendererCore::GetRenderer();
        descriptorPool = std::make_unique<vpr::DescriptorPool>(renderer.Device(), groupToIdxMap.size());
        const auto& rsrc_counts = shaderPack->GetTotalDescriptorTypeCounts();
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, rsrc_counts.UniformBuffers);
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, rsrc_counts.UniformBuffersDynamic);
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, rsrc_counts.StorageBuffers);
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, rsrc_counts.StorageBuffersDynamic);
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, rsrc_counts.StorageTexelBuffers);
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, rsrc_counts.UniformTexelBuffers);
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, rsrc_counts.StorageImages);
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_SAMPLER, rsrc_counts.Samplers);
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, rsrc_counts.SampledImages);
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, rsrc_counts.CombinedImageSamplers);
        descriptorPool->Create();
    }

    constexpr static bool is_buffer_type(const VkDescriptorType type) {
        if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
            type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER || type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
            return true;
        }
        else {
            return false;
        }
    }

    constexpr static bool is_texel_buffer(const VkDescriptorType type) {
        if (type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER || type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) {
            return true;
        }
        else {
            return false;
        }
    }

    void ShaderResourcePack::createSingleSet(const std::string & name) {

        size_t num_resources = 0;
        shaderPack->GetResourceGroupPointers(name.c_str(), &num_resources, nullptr);
        std::vector<const st::ShaderResource*> resources(num_resources);
        shaderPack->GetResourceGroupPointers(name.c_str(), &num_resources, resources.data());
        auto& buffer_cache = graph.BufferResourceCache();
        // TODO: Non buffer resource cache object
        buffer_cache.AddResources(resources);
        
        const size_t& idx = groupToIdxMap.at(name);
        descriptorSets[idx] = std::move(std::make_unique<vpr::DescriptorSet>(graph.GetDevice()));
        for (const auto& rsrc : resources) {
            if (is_buffer_type(rsrc->GetType())) {
                vpr::Buffer* buffer = buffer_cache.at(name.c_str());
                //descriptorSets[idx]->AddDescriptorInfo(buffer->GetDescriptor(), rsrc->GetType(), rsrc->)
            }
            else if (is_texel_buffer(rsrc->GetType())) {

            }
        }
    }

    void ShaderResourcePack::createSets() {
        for (const auto& entry : groupToIdxMap) {
            createSingleSet(entry.first);
        }
    }

}
