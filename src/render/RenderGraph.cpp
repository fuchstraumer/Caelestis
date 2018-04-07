#include "render/RenderGraph.hpp"

namespace vpsk {

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

}