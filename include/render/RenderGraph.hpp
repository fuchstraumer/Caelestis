#pragma once
#ifndef VPSK_RENDERGRAPH_HPP
#define VPSK_RENDERGRAPH_HPP
#include "ForwardDecl.hpp"
#include "RenderResource.hpp"
#include <unordered_map>

namespace vpsk {

    class RenderGraph {
        RenderGraph(const RenderGraph&) = delete;
        RenderGraph& operator=(const RenderGraph&) = delete;
    public:

        const ImageResource& GetImageResource(const std::string& name);
        const BufferResource& GetBufferResource(const std::string& name);
    private:

        std::unordered_map<std::string, size_t> resourcesIdxMap;
        std::vector<subpass_resource_t> resources;

    };

}

#endif //!VPSK_RENDERGRAPH_HPP