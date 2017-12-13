#pragma once
#ifndef VPSK_DEPTH_PREPASS_HPP
#define VPSK_DEPTH_PREPASS_HPP
#include "BasePass.hpp"
namespace vpsk {

    /**The DepthPrePass can be used to see what objects and primitives being currently rendered 
     * are actually visible on the screen. The output is saved and can be accessed at a later time.
     */
    class DepthPrePass {
    public:
        
    private:

        void setAttachmentData();
        void setSubpassData();
        void createRenderpass();
        void createPipelineLayout();
        void setPipelineState();
        void createGraphicsPipeline();

        VkAttachmentDescription depthDescr;
        VkSubpassDescription subpassDescr;
        VkSubpassDependency subpassDependency;
        VkAttachmentReference depthRef;

        std::unique_ptr<vpr::Renderpass> renderpass;
        std::unique_ptr<vpr::ShaderModule> vert, frag;
        std::unique_ptr<vpr::PipelineLayout> layout;
        std::map<std::type_index, vpr::GraphicsPipeline> pipelines;
        const vpr::Device* dvc;
    };

}

#endif //!VPSK_DEPTH_PREPASS_HPP