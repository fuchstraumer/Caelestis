#pragma once
#ifndef VPSK_SUBPASS_HPP
#define VPSK_SUBPASS_HPP

#include "ForwardDecl.hpp"
#include "RenderResource.hpp"
#include "util/Delegate.hpp"
#include <vector>

namespace vpsk {

    class RenderPass;

    class Subpass {
        Subpass(const Subpass&) = delete;
        Subpass& operator=(const Subpass&) = delete;
    public:
        Subpass(const std::string& name, VkPipelineStageFlags flags);

        void RecordCommands(const VkCommandBuffer& cmd);
        bool GetAttachmentClearColor(const size_t idx, VkClearColorValue* dest = nullptr);
        bool GetDepthClearValue(VkClearDepthStencilValue* dest = nullptr);
        
        std::string Name;
        const RenderPass* ParentRenderpass = nullptr;
        ImageResource* DepthInput = nullptr;
        ImageResource* DepthOutput = nullptr;

        std::vector<ImageResource*> ColorOutputs;
        std::vector<ImageResource*> ColorInputs;
        std::vector<ImageResource*> ResolveOutputs;
        std::vector<ImageResource*> TextureInputs;
        std::vector<ImageResource*> AttachmentInputs;
        std::vector<BufferResource*> UniformInputs;

    private:
        size_t indexInRenderpass;
        delegate_t<void(const VkCommandBuffer&)> renderpassCallback;
        VkPipelineStageFlags stages;
    };

}

#endif //!VPSK_SUBPASS_HPP