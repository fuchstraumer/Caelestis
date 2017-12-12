#include "renderpasses/BasePass.hpp"
using namespace vpr;
namespace vpsk {

    void BasePass::RegisterFeature(const std::type_index& idx, const GraphicsPipelineInfo& info) {
        VkGraphicsPipelineCreateInfo info = info.GetPipelineCreateInfo();
    }
}