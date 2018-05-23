#include "render/RenderGraph.hpp"

namespace vpsk {

    RenderGraph::RenderGraph(const vpr::Device * dvc) : device(dvc), descriptorResources(std::make_unique<DescriptorResources>(dvc)) {}

    PipelineSubmission & RenderGraph::AddSubmission(const std::string & name, VkPipelineStageFlags stages) {
        auto iter = submissions.emplace(name, std::make_unique<PipelineSubmission>(*this, name, 0, stages));
        return *(iter.first->second);
    }

    PipelineResource& RenderGraph::GetResource(const std::string & name) {
        auto iter = resourceNameMap.find(name);
        if (iter != resourceNameMap.cend()) {
            const size_t idx = iter->second;
            return *resources[idx];
        }
        else {
            const size_t idx = resources.size();
            resources.emplace_back(std::make_unique<PipelineResource>(name, idx));
            resourceNameMap[name] = idx;
            return *resources.back();
        }
    }

    const vpr::Device* RenderGraph::GetDevice() const noexcept {
        return device;
    }

    DescriptorResources * RenderGraph::GetDescriptorResources() {
        return descriptorResources.get();
    }

}