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
        virtual ~BasePass(){};
        virtual void RegisterFeature(const std::type_index& idx, const vpr::GraphicsPipelineInfo& info, 
            const VkPipelineLayout& layout, render_function_t func) = 0;
    private:
        std::map<std::type_index, std::unique_ptr<vpr::GraphicsPipeline>> pipelines;
        std::map<std::type_index, render_function_t> renderFunctions;
        std::unique_ptr<Renderpass> renderpass;
        std::unique_ptr<PipelineCache> cache;
        VkGraphicsPipelineCreateInfo createInfo;
    };

}


#endif //!VPSK_BASE_PASS_HPP