#include "graph/RenderGraph.hpp"
#include "graph/PipelineResource.hpp"
#include "resource/DescriptorSet.hpp"
#include "systems/BufferResourceCache.hpp"
#include "systems/ImageResourceCache.hpp"

namespace vpsk {

    RenderGraph::RenderGraph(const vpr::Device * dvc) : device(dvc) {
        bufferResources = std::make_unique<vpsk::BufferResourceCache>(device);
        imageResources = std::make_unique<vpsk::ImageResourceCache>(device);
    }

    RenderGraph::~RenderGraph()
    {
    }

    PipelineResource& RenderGraph::GetResource(const std::string& name) {
        auto iter = resourceNameMap.find(name);
        if (iter != resourceNameMap.cend()) {
            const size_t& idx = iter->second;
            return *resources[idx];
        }
        else {
            const size_t idx = resources.size();
            resources.emplace_back(std::make_unique<PipelineResource>(name, idx));
            resourceNameMap[name] = idx;
            return *resources.back();
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
