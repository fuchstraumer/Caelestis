#pragma once
#ifndef VULPESCENEKIT_PRIMITIVE_HPP
#define VULPESCENEKIT_PRIMITIVE_HPP

#include <vulkan/vulkan.h>

namespace vpsk {

    class Primitive {
    public:
        Primitive() = default;
        virtual ~Primitive(){};
        virtual void Render(const VkCommandBuffer& cmd_buffer, const VkCommandBufferBeginInfo& begin_info, const VkViewport& curr_viewport, const VkRect2D& curr_scissor) = 0;
    };

}

#endif //!VULPESCENEKIT_PRIMITIVE_HPP