#include "renderpasses/PhongPass.hpp"
#include "render/Multisampling.hpp"
#include "render/Renderpass.hpp"
#include "resource/PipelineCache.hpp"

using namespace vpr;

namespace vpsk {

    void PhongPass::createRenderTargetAttachment() {
        attachmentDescriptions[0] = vk_attachment_description_base;
        attachmentDescriptions[0].format = colorFormat;
        attachmentDescriptions[0].samples = msaaSamples;
        attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    
    void PhongPass::createResolveAttachment() {
        attachmentDescriptions[1] = vk_attachment_description_base;
        attachmentDescriptions[1].format = colorFormat;
        attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }

    void PhongPass::createMultisampleDepthAttachment() {
        attachmentDescriptions[2] = vk_attachment_description_base;
        attachmentDescriptions[2].format = device->FindDepthFormat();
        attachmentDescriptions[2].samples = BaseScene::SceneConfiguration.MSAA_SampleCount;
        attachmentDescriptions[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescriptions[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescriptions[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescriptions[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescriptions[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDescriptions[2].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    void PhongPass::createResolveDepthAttachment() {

        attachmentDescriptions[3] = vk_attachment_description_base;
        attachmentDescriptions[3].format = device->FindDepthFormat();
        attachmentDescriptions[3].samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescriptions[3].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescriptions[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescriptions[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescriptions[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescriptions[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDescriptions[3].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

}