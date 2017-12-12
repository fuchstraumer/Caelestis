#pragma once
#ifndef VPSK_BASE_PASS_HPP
#define VPSK_BASE_PASS_HPP
#include "ForwardDecl.hpp"
#include "render/GraphicsPipeline.hpp"
#include <typeinfo>
#include <typeindex>
#include <map>

namespace vpsk {

    using render_function_t = std::function<void(const VkCommandBuffer&)>;

    class BasePass {
    public:
        void RegisterFeature(const std::type_index& idx, const vpr::GraphicsPipelineInfo& info, render_function_t func);
    private:

        std::map<std::type_index, vpr::GraphicsPipeline> pipelines;
        std::map<std::type_index, render_function_t> renderFunctions;
    };

}


#endif //!VPSK_BASE_PASS_HPP