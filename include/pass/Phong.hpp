#pragma once
#ifndef VPSK_BASIC_PHONG_PASS_HPP
#define VPSK_BASIC_PHONG_PASS_HPP
#include <vulkan/vulkan.h>
#include <functional>
#include <vector>
#include <mutex>
#include <map>
#include "util/Delegate.hpp"
#include "render/Multisampling.hpp"
namespace vpsk {

    class PhongPass {
        PhongPass(const PhongPass&) = delete;
        PhongPass& operator=(const PhongPass&) = delete;
    public:

        PhongPass(const vpr::Device* dvc, const vpr::Swapchain* swap);
        VkCommandBuffer RenderFrame(const size_t idx, VkFramebuffer fbuff) const;
        void RegisterFeature(std::string name, const vpr::GraphicsPipelineInfo& info, VkPipelineLayout layout);
        void SetFeatureRenderFn(std::string name, delegate_t<void(VkCommandBuffer)> func);
        // This has to be delayed until you have registered features
        void Construct();

    private:

        void secondaryRender(const size_t idx, const VkCommandBufferBeginInfo s_info) const;

        void createRenderTargetAttachment();
        void createResolveAttachment();
        void createDepthAttachment();
        void createDepthResolveAttachment();
        void createAttachmentReferences();
        void createSubpassDescription();
        void createSubpassDependencies();
        void createRenderpass();

        void createDepthStencil();
        void createPrimaryPool();
        void createSecondaryPool();

        std::unique_ptr<vpr::Multisampling> msaa;
        std::unique_ptr<vpr::DepthStencil> depthStencil;

        VkSubpassDescription sbDescr;
        std::array<VkAttachmentDescription, 4> attachmentDescriptions;
        std::array<VkAttachmentReference, 3> attachmentReferences;
        std::array<VkSubpassDependency, 2> subpassDependencies;
        std::unique_ptr<vpr::Renderpass> renderpass;

        std::map<std::string, size_t> nameToIdxMap;
        std::map<size_t, delegate_t<void(VkCommandBuffer)>> renderFunctions;
        std::unique_ptr<vpr::PipelineCache> cache;
        std::map<size_t, std::unique_ptr<vpr::GraphicsPipeline>> pipelines;
        std::map<size_t, VkGraphicsPipelineCreateInfo> pipelineInfos;
        std::unique_ptr<vpr::CommandPool> primaryPool;
        std::unique_ptr<vpr::CommandPool> secondaryPool;
        const vpr::Swapchain* swapchain;
        const vpr::Device* device;
        bool constructed;
    };

}

#endif //!VPSK_BASIC_PHONG_PASS_HPP