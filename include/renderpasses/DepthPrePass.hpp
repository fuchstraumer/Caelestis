#pragma once
#ifndef VPSK_DEPTH_PREPASS_HPP
#define VPSK_DEPTH_PREPASS_HPP
#include "ForwardDecl.hpp"
#include <vulkan/vulkan.h>
namespace vpsk {

    /**
     * The renderpasses group covers fully setup renderpass wrapper structs 
     * entailing different kinds of rendering passes performed during the process
     * of rendering a frame. This can include things like the DepthPrepass for 
     * lighting and culling assistance, or the more complete PBR/G-buffer related
     * pass setups.
     * \defgroup Renderpasses
    */ 


    /**The DepthPrePass can be used to see what objects and primitives being currently rendered 
     * are actually visible on the screen.
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
        std::unique_ptr<vpr::GraphicsPipeline> pipeline;
        std::unique_ptr<vpr::ShaderModule> vert, frag;
        std::unique_ptr<vpr::PipelineLayout> layout;

        const vpr::Device* dvc;
    };

}

#endif //!VPSK_DEPTH_PREPASS_HPP