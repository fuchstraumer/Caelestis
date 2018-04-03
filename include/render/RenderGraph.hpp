#pragma once
#ifndef VPSK_RENDERGRAPH_HPP
#define VPSK_RENDEGRAPH_HPP
#include "resources/PipelineResource.hpp"
#include <unordered_map>

namespace vpsk {

    class RenderGraph {
    public:

    private:

        using barrier_variant_t = std::variant<VkBufferMemoryBarrier, VkImageMemoryBarrier, VkMemoryBarrier>;
        struct resource_barriers_t { 
            std::vector<barrier_variant_t> Invalidates;
            std::vector<barrier_variant_t> Flushes;
        };
        std::unordered_map<std::string, vpr::Buffer*> usedBuffers;
        std::unordered_map<std::string, vpr::Image*> usedImages;
        std::unordered_map<std::string, PipelineResource> resources;
    };

}
#endif // !VPSK_RENDERGRAPH_HPP
