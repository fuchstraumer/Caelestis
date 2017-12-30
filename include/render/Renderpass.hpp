#pragma once
#ifndef VPSK_RENDERPASS_HPP
#define VPSK_RENDERPASS_HPP

#include "ForwardDecl.hpp"
#include "Subpass.hpp"
#include "ShaderGroup.hpp"
#include <unordered_map>

namespace vpsk {

    class Renderpass {
        Renderpass(const Renderpass&) = delete;
        Renderpass& operator=(const Renderpass&) = delete;
    public:

        Renderpass(const vpr::Device* dvc);

        Subpass& AddSubpass(const std::string& name, VkPipelineStageFlags stages);

        ImageResource& GetImageResource(const std::string& name);
        BufferResource& GetBufferResource(const std::string& name);

    private:

        std::vector<Subpass> subpasses;
        // Subpass 0 uses group 0, 1 uses 1, etc
        std::vector<ShaderGroup> shaders;
        std::unordered_map<std::string, subpass_resource_t> resources;
        std::unordered_map<std::string, size_t> subpassNameIdxMap
    };

}

#endif //!VPSK_RENDERPASS_HPP