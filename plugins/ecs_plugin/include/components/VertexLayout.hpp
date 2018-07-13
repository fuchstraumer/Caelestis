#pragma once
#ifndef VPSK_VERTEX_LAYOUT_HPP
#define VPSK_VERTEX_LAYOUT_HPP
#include <vulkan/vulkan.h>
#include <vector>

namespace vpsk {

    // Standard vertex layout:
    // vec3 Position
    // --- new buffer ---
    // vec3 Normal
    // vec3 UV
    // vec3 Tangent
    // vec3 Bitangent
    // --- new buffer --- 
    // vec3 boneweights
    // vec2 boneweights2
    // ivec3 boneIDs
    // ivec2 boneIDs2

    static constexpr VkVertexInputBindingDescription StandardBindings[3] {
        VkVertexInputBindingDescription{ 0, sizeof(float) * 3, VK_VERTEX_INPUT_RATE_VERTEX },
        VkVertexInputBindingDescription{ 1, sizeof(float) * 12, VK_VERTEX_INPUT_RATE_VERTEX },
        VkVertexInputBindingDescription{ 2, sizeof(float) * 10, VK_VERTEX_INPUT_RATE_VERTEX }
    };

    enum class LayoutType {
        Standard,
        AnimatedMesh
    };

    struct VertexLayoutComponent {
        VertexLayoutComponent(std::vector<VkVertexInputAttributeDescription> attrs, const LayoutType layout = LayoutType::Standard) : Attributes(std::move(attrs)),
            Bindings((layout == LayoutType::Standard) ? 
                std::vector<VkVertexInputBindingDescription>{ &StandardBindings[0], &StandardBindings[0] + 2 } :
                std::vector<VkVertexInputBindingDescription>{ &StandardBindings[0], &StandardBindings[0] + 3 }) {}
        VertexLayoutComponent(std::vector<VkVertexInputAttributeDescription> attrs, std::vector<VkVertexInputBindingDescription> binds) : 
            Attributes(std::move(attrs)), Bindings(std::move(binds)) {}
        const std::vector<VkVertexInputAttributeDescription> Attributes;
        const std::vector<VkVertexInputBindingDescription> Bindings;
    };

}

#endif // !VPSK_VERTEX_LAYOUT_HPP
