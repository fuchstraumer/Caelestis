#pragma once
#ifndef VPSK_DYNAMIC_STATE_COMPONENT_HPP
#define VPSK_DYNAMIC_STATE_COMPONENT_HPP
#include <vulkan/vulkan.h>
#include <vector>

namespace vpsk {

    struct DynamicViewportStateComponent {
        DynamicViewportStateComponent(std::vector<VkViewport> ports, const uint32_t idx = 0) : FirstViewport(idx), Viewports(std::move(ports)) {}
        uint32_t FirstViewport{ 0 };
        std::vector<VkViewport> Viewports;
        void operator()(VkCommandBuffer cmd) {
            vkCmdSetViewport(cmd, FirstViewport, static_cast<uint32_t>(Viewports.size()), Viewports.data());
        }
    };

    struct DynamicScissorStateComponent {
        DynamicScissorStateComponent(std::vector<VkRect2D> rects, const uint32_t idx = 0) : FirstScissor(idx), Scissors(rects) {}
        uint32_t FirstScissor{ 0 };
        std::vector<VkRect2D> Scissors;
        void operator()(VkCommandBuffer cmd) {
            vkCmdSetScissor(cmd, FirstScissor, static_cast<uint32_t>(Scissors.size()), Scissors.data());
        }
    };

}

#endif //!VPSK_DYNAMIC_STATE_COMPONENT_HPP