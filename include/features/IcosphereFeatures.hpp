#pragma once
#ifndef TRIANGLE_FEATURE_HPP
#define TRIANGLE_FEATURE_HPP
#include "geometries/Icosphere.hpp"
#include <set>
namespace vpsk {

    class IcosphereFeatures {
        IcosphereRenderer(const IcosphereRenderer&) = delete;
        IcosphereRenderer(const IcosphereRenderer&) = delete;
    public:
        
        void Render(const VkCommandBuffer& cmd, const VkCommandBufferBeginInfo& begin, const VkViewport& viewport, const VkRect2D& rect);

    private:

        
    };

}

#endif