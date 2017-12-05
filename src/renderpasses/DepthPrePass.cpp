#include "renderpasses/DepthPrePass.hpp"
#include "core/LogicalDevice.hpp"

using namespace vpr;

namespace vpsk {

    void DepthPrePass::setAttachmentData() {

        depthDescr = vk_attachment_description_base;
        depthDescr.format = dvc->FindDepthFormat();
        depthDescr.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthDescr.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthDescr.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthDescr.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthDescr.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthDescr.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

        depthRef = VkAttachmentReference{ 0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    }

    void DepthPrePass::setSubpassData() {

        subpassDescr = vk_subpass_description_base;
        subpassDescr.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescr.colorAttachmentCound = 0;
        subpassDescr.pDepthStencilAttachment = &depthRef;

        subpassDependency = vk_subpass_dependency_base;
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0; // 0 refers to this subpass.
        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    }

    void DepthPrePass::createRenderpass() {

        VkRenderPassCreateInfo rp_info = vk_render_pass_create_info_base;
        rp_info.attachmentCount = 1;
        rp_info.pAttachments = &depthDescr;
        rp_info.subpassCount = 1;
        rp_info.pSubpasses = &subpassDescr;
        rp_info.dependencyCount = 1;
        rp_info.pDependencies = &subpassDependency;

        renderPass = std::make_unique<vpr::Renderpass>(dvc, rp_info);

    }

    void DepthPrePass::createPipelineLayout() {

        std::vector<VkDescriptorSetLayout> sets { 0, 0 };
        layout = std::make_unique<PipelineLayout>(dvc);
        layout->Create(sets);
        
    }

    void DepthPrePass::setPipelineState() {

        GraphicsPipelineInfo gpinfo;
        constexpr static VkDynamicState dynstates[2] { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        gpinfo.DynamicStateInfo.dynamicStateCount = 2;
        gpinfo.DynamicStateInfo.pDynamicStates = dynstates;
        gpInfo.DepthStencilInfo.depthWriteEnable = VK_TRUE;

    }

}