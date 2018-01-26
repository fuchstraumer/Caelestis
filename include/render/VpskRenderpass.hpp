#pragma once
#ifndef VPSK_RENDERPASS_HPP
#define VPSK_RENDERPASS_HPP
#include "ForwardDecl.hpp"
#include "Subpass.hpp"
#include <unordered_map>

namespace vpsk {

    class RenderPass {
        RenderPass(const RenderPass&) = delete;
        RenderPass& operator=(const RenderPass&) = delete;
    public:

        RenderPass(const vpr::Device* dvc = nullptr);

        void LoadFromJSON(const std::string& input_file);

        Subpass& AddSubpass(const std::string& name, VkPipelineStageFlags stages);

        ImageResource& GetImageResource(const std::string& name);
        BufferResource& GetBufferResource(const std::string& name);

        ImageResource& SetDepthStencilInput(const std::string& name);
        ImageResource& SetDepthStencilOutput(const std::string& name);
        
    private:
        const vpr::Device* device;
        std::string name;
        std::vector<Subpass> subpasses;
        std::unordered_map<std::string, subpass_resource_t> resources;
        std::unordered_map<std::string, size_t> subpassNameIdxMap;
    };

}

#endif //!VPSK_RENDERPASS_HPP