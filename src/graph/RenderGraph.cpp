#include "graph/RenderGraph.hpp"
#include "graph/PipelineResource.hpp"
#include "resource/DescriptorSet.hpp"
#include "systems/BufferResourceCache.hpp"
#include "systems/ImageResourceCache.hpp"
#include "core/ShaderPack.hpp"
#include "core/ShaderResource.hpp"

namespace vpsk {

    RenderGraph::RenderGraph(const vpr::Device * dvc) : device(dvc) {
        bufferResources = std::make_unique<vpsk::BufferResourceCache>(device);
        imageResources = std::make_unique<vpsk::ImageResourceCache>(device);
    }

    RenderGraph::~RenderGraph() {}

    PipelineResource& RenderGraph::GetResource(const std::string& name) {
        auto iter = resourceNameMap.find(name);
        if (iter != resourceNameMap.cend()) {
            const size_t& idx = iter->second;
            return *pipelineResources[idx];
        }
        else {
            const size_t idx = pipelineResources.size();
            pipelineResources.emplace_back(std::make_unique<PipelineResource>(name, idx));
            resourceNameMap[name] = idx;
            return *pipelineResources.back();
        }
    }

    void RenderGraph::AddShaderPackResources(const st::ShaderPack* pack) {

        std::vector<std::string> resource_group_names;
        {
            st::dll_retrieved_strings_t names = pack->GetResourceGroupNames();
            for (size_t i = 0; i < names.NumStrings; ++i) {
                resource_group_names.emplace_back(names.Strings[i]);
            }
        }

        std::unordered_map<std::string, std::vector<const st::ShaderResource*>> resources;
        
        for (auto& name : resource_group_names) {
            size_t num_rsrc = 0;
            pack->GetResourceGroupPointers(name.c_str(), &num_rsrc, nullptr);
            std::vector<const st::ShaderResource*> group(num_rsrc);
            pack->GetResourceGroupPointers(name.c_str(), &num_rsrc, group.data());
            resources.emplace(name.c_str(), group);
        }

        for (auto& group : resources) {
            imageResources->AddResources(group.second);
            bufferResources->AddResources(group.second);
        }
        
    }

    BufferResourceCache& RenderGraph::GetBufferResourceCache() {
        return *bufferResources;
    }

    ImageResourceCache& RenderGraph::GetImageResourceCache() {
        return *imageResources;
    }

    const vpr::Device* RenderGraph::GetDevice() const noexcept {
        return device;
    }

}
