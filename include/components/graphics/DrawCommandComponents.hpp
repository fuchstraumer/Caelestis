#pragma once
#ifndef VPSK_DRAW_COMMAND_COMPONENTS_HPP
#define VPSK_DRAW_COMMAND_COMPONENTS_HPP
#include <vulkan/vulkan.h>
namespace vpsk {

    struct DrawCommandComponent {
        uint32_t VertexCount;
        uint32_t InstanceCount;
        uint32_t FirstVertex;
        uint32_t FirstInstance;
        void operator()(VkCommandBuffer cmd) {
            vkCmdDraw(cmd, VertexCount, InstanceCount, FirstVertex, FirstInstance);
        }
    };

    struct IndexedDrawCommandComponent {
        uint32_t IndexCount;
        uint32_t InstanceCount;
        uint32_t FirstIndex;
        uint32_t VertexOffset;
        uint32_t FirstInstance;
        void operator()(VkCommandBuffer cmd) {
            vkCmdDrawIndexed(cmd, IndexCount, InstanceCount, FirstIndex, VertexOffset, FirstInstance);
        }
    };

    struct DrawIndirectCommandComponent {
    };

}

#endif //!VPSK_DRAW_COMMAND_COMPONENTS_HPP
