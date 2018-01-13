#include "pass/Phong.hpp"
#include "render/Swapchain.hpp"
#include "scene/BaseScene.hpp"
#include "core/LogicalDevice.hpp"
#include "render/Renderpass.hpp"
#include "render/DepthStencil.hpp"
#include "command/CommandPool.hpp"
#include "resource/PipelineCache.hpp"
#include "util/easylogging++.h"
#include <typeinfo>
#include <future>

namespace vpsk {

    PhongPass::PhongPass(const vpr::Device* dvc, const vpr::Swapchain* swap) : device(dvc), swapchain(swap) {
        createDepthStencil();
        createRenderpass();
    }

    VkCommandBuffer PhongPass::RenderFrame(const size_t idx) const {
        if (!constructed) {
            LOG(ERROR) << "Didn't construct PhongPass before attempting use: expect delay as construction is now performed.";
            throw std::runtime_error("Failed to construct PhongPass before usage.");
        }

        const VkCommandBufferBeginInfo primary_begin_info{
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            nullptr,
            VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            nullptr
        };

        const VkCommandBufferInheritanceInfo sc_inherit_info{
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
            nullptr,
            renderpass->vkHandle(),
            0,
            framebuffers[idx],
            VK_FALSE,
            0,
            0
        };

        const VkCommandBufferBeginInfo secondary_begin_info{
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            nullptr,
            VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            &sc_inherit_info
        };

        const auto get_idx = [&](const size_t frame_idx, const size_t obj_idx) {
            return (idx * frame_idx) + obj_idx;
        };

        renderpass->UpdateBeginInfo(framebuffers[idx]);

        VkResult err = vkBeginCommandBuffer(primaryPool->GetCmdBuffer(idx), &primary_begin_info); VkAssert(result);
            vkCmdBeginRenderPass(primaryPool->GetCmdBuffer(idx), &renderpass->BeginInfo(), VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
            secondaryRender(idx, secondary_begin_info);
            vkCmdExecuteCommands(primaryPool->GetCmdBuffer(idx), static_cast<uint32_t>(pipelines.size()), secondaryPool->GetCommandBuffers(idx * primaryPool->size()));
            vkCmdEndRenderPass(primaryPool->GetCmdBuffer(idx));
        err = vkEndCommandBuffer(primaryPool->GetCmdBuffer(idx)); VkAssert(result);

        return primaryPool->GetCmdBuffer(idx);
    }

    void PhongPass::RegisterFeature(std::string name, const vpr::GraphicsPipelineInfo& info, VkPipelineLayout layout) {
        nameToIdxMap.emplace(name, nameToIdxMap.size() - 1);
        pipelineInfos.emplace(nameToIdxMap.at(name), info.GetPipelineCreateInfo());
        pipelineInfos[nameToIdxMap.at(name)].layout = layout;
        pipelineInfos[nameToIdxMap.at(name)].renderPass = renderpass->vkHandle();
        pipelineInfos[nameToIdxMap.at(name)].basePipelineIndex = -1;
    }

    void PhongPass::SetFeatureRenderFn(std::string name, delegate_t<void(VkCommandBuffer)> func) {
        renderFunctions.emplace(nameToIdxMap.at(name), std::move(func));
    }

    void PhongPass::Construct() {
        createPrimaryPool();
        createSecondaryPool();
        cache = std::make_unique<vpr::PipelineCache>(device, static_cast<uint16_t>(typeid(PhongPass).hash_code()));

        std::vector<VkGraphicsPipelineCreateInfo> infos;

        for(auto& info : pipelineInfos) {
            VkPipeline new_handle = VK_NULL_HANDLE;
            VkResult result = vkCreateGraphicsPipelines(device->vkHandle(), cache->vkHandle(), 1, &info.second, nullptr, &new_handle);
            pipelines.emplace(info.first, std::make_unique<vpr::GraphicsPipeline>(device, info.second, new_handle));
        }
        
    }

    void PhongPass::secondaryRender(const size_t idx, const VkCommandBufferBeginInfo s_info) const {
        std::vector<std::future<void>> render_futures;
        for (auto& pline : pipelines) {
            vkBeginCommandBuffer(secondaryPool->GetCmdBuffer((idx * primaryPool->size()) + pline.first), &s_info);
            vkCmdBindPipeline(secondaryPool->GetCmdBuffer((idx * primaryPool->size()) + pline.first), VK_PIPELINE_BIND_POINT_GRAPHICS, pline.second->vkHandle());
            render_futures.push_back(std::async(std::launch::async, renderFunctions.at(pline.first), (secondaryPool->GetCmdBuffer((idx * primaryPool->size()) + pline.first))));
        }

        for (auto& fut : render_futures) {
            fut.get();
        }

    }

    void PhongPass::createRenderTargetAttachment() {
        attachmentDescriptions[0] = vpr::vk_attachment_description_base;
        attachmentDescriptions[0].format = swapchain->ColorFormat();
        attachmentDescriptions[0].samples = BaseScene::SceneConfiguration.MSAA_SampleCount;
        attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    void PhongPass::createResolveAttachment() {
        attachmentDescriptions[1] = vpr::vk_attachment_description_base;
        attachmentDescriptions[1].format = attachmentDescriptions[0].format;
        attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }

    void PhongPass::createDepthAttachment() {
        attachmentDescriptions[2] = vpr::vk_attachment_description_base;
        attachmentDescriptions[2].format = device->FindDepthFormat();
        attachmentDescriptions[2].samples = BaseScene::SceneConfiguration.MSAA_SampleCount;
        attachmentDescriptions[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescriptions[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescriptions[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescriptions[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescriptions[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDescriptions[2].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    void PhongPass::createDepthResolveAttachment() {
        attachmentDescriptions[3] = vpr::vk_attachment_description_base;
        attachmentDescriptions[3].format = attachmentDescriptions[1].format;
        attachmentDescriptions[3].samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescriptions[3].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescriptions[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescriptions[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescriptions[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescriptions[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDescriptions[3].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    void PhongPass::createAttachmentReferences() {
        attachmentReferences[0] = VkAttachmentReference{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        attachmentReferences[1] = VkAttachmentReference{ 2, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
        attachmentReferences[2] = VkAttachmentReference{ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    }

    void PhongPass::createSubpassDescription() {
        sbDescr.colorAttachmentCount = 1;
        sbDescr.pColorAttachments = &attachmentReferences[0];
        sbDescr.pResolveAttachments = &attachmentReferences[2];
        sbDescr.pDepthStencilAttachment = &attachmentReferences[1];
    }   

    void PhongPass::createSubpassDependencies() {
        subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependencies[0].dstSubpass = 0;
        subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        subpassDependencies[1].srcSubpass = 0;
        subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    }

    void PhongPass::createRenderpass() {
        msaa = std::make_unique<vpr::Multisampling>(device, swapchain, BaseScene::SceneConfiguration.MSAA_SampleCount);

        createRenderTargetAttachment();
        createResolveAttachment();
        createDepthAttachment();
        createResolveAttachment();
        createAttachmentReferences();
        createSubpassDescription();
        createSubpassDependencies();

        VkRenderPassCreateInfo rp_info = vpr::vk_render_pass_create_info_base;
        rp_info.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
        rp_info.pAttachments = attachmentDescriptions.data();
        rp_info.subpassCount = 1;
        rp_info.pSubpasses = &sbDescr;
        rp_info.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
        rp_info.pDependencies = subpassDependencies.data();

        renderpass = std::make_unique<vpr::Renderpass>(device, rp_info);

        const static std::vector<VkClearValue> clear_values {
            { 0.25f, 0.25f, 0.25f, 1.0f },
            { 0.25f, 0.25f, 0.25f, 1.0f },
            { 1.0f, 0}
        };

        renderpass->SetupBeginInfo(clear_values.data(), clear_values.size(), swapchain->Extent());

    }

    void PhongPass::createFramebuffers() {
        std::array<VkImageView, 4> attachments{ msaa->ColorBufferMS->View(), VK_NULL_HANDLE, msaa->DepthBufferMS->View(), depthStencil->View() };
        VkFramebufferCreateInfo framebuffer_create_info = vpr::vk_framebuffer_create_info_base;
        framebuffer_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebuffer_create_info.pAttachments = attachments.data();
        framebuffer_create_info.renderPass = renderpass->vkHandle();
        framebuffer_create_info.width = swapchain->Extent().width;
        framebuffer_create_info.height = swapchain->Extent().height;
        framebuffer_create_info.layers = 1;

        for (uint32_t i = 0; i < swapchain->ImageCount(); ++i) {
            attachments[1] = swapchain->ImageView(i);
            VkFramebuffer new_fbuff;
            VkResult result = vkCreateFramebuffer(device->vkHandle(), &framebuffer_create_info, nullptr, &new_fbuff);
            VkAssert(result);
            framebuffers.push_back(std::move(new_fbuff));
        }
    }
    
    void PhongPass::createDepthStencil() {
        depthStencil = std::make_unique<vpr::DepthStencil>(device, VkExtent3D{ swapchain->Extent().width, swapchain->Extent().height, 1});
    }

    void PhongPass::createPrimaryPool() {
        VkCommandPoolCreateInfo pool_info = vpr::vk_command_pool_info_base;
        pool_info.queueFamilyIndex = device->QueueFamilyIndices.Graphics;
        primaryPool = std::make_unique<vpr::CommandPool>(device, pool_info);
        primaryPool->AllocateCmdBuffers(swapchain->ImageCount(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    }

    void PhongPass::createSecondaryPool() {
        VkCommandPoolCreateInfo pool_info = vpr::vk_command_pool_info_base;
        pool_info.queueFamilyIndex = device->QueueFamilyIndices.Graphics;
        secondaryPool = std::make_unique<vpr::CommandPool>(device, pool_info);
        secondaryPool->AllocateCmdBuffers(swapchain->ImageCount() * static_cast<uint32_t>(pipelines.size()), VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    }

}