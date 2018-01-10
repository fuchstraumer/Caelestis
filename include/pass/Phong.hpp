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
    public:

        void RenderFrame();
    private:

        std::mutex renderMutex;
        std::vector<delegate_t<void(VkCommandBuffer)>> renderFunctions;
        std::unique_ptr<vpr::Multisampling> msaa;
        std::unique_ptr<vpr::DepthStencil> depthStencil;

        std::array<VkAttachmentDescription, 4> attachmentDescriptions;
        std::array<VkAttachmentReference, 3> attachmentReferences;
        std::array<VkSubpassDependency, 2> dependencies;
        std::unique_ptr<vpr::Renderpass> renderpass;

        std::multimap<std::string, std::unique_ptr<vpr::PipelineCache>> caches;
        std::multimap<std::string, std::unique_ptr<vpr::GraphicsPipeline>> pipelines;

        std::unique_ptr<vpr::CommandPool> primaryPool;
        std::unique_ptr<vpr::CommandPool> secondaryPool;
    };

}

#endif //!VPSK_BASIC_PHONG_PASS_HPP