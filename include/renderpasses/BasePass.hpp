#pragma once
#ifndef VPSK_BASE_PASS_HPP
#define VPSK_BASE_PASS_HPP
#include "ForwardDecl.hpp"
#include "render/GraphicsPipeline.hpp"
#include <typeinfo>
#include <map>

namespace vpsk {

    class BasePass {
    public:
        void RegisterFeature(const std::type_index& idx, const vpr::GraphicsPipelineInfo& info);
    private:

        std::map<std::type_index, vpr::GraphicsPipeline> pipelines;
    };

}


#endif //!VPSK_BASE_PASS_HPP