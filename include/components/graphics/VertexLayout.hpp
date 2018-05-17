#pragma once
#ifndef VPSK_VERTEX_LAYOUT_HPP
#define VPSK_VERTEX_LAYOUT_HPP
#include <vulkan/vulkan.h>
#include <vector>

namespace vpsk {

    static const std::vector<VkVertexInputBindingDescription> StandardBindings {
        VkVertexInputBindingDescription{ 0, sizeof(float) * 3, VK_VERTEX_INPUT_RATE_VERTEX },
        VkVertexInputBindingDescription{ 0, sizeof(float) * 8, VK_VERTEX_INPUT_RATE_VERTEX }
    };

    struct VertexLayoutComponent {
        VertexLayoutComponent(std::vector<VkVertexInputAttributeDescription> attrs) : Attributes(std::move(attrs)), Bindings(StandardBindings) {}
        VertexLayoutComponent(std::vector<VkVertexInputAttributeDescription> attrs, std::vector<VkVertexInputBindingDescription> binds) : 
            Attributes(std::move(attrs)), Bindings(std::move(binds)) {}
        const std::vector<VkVertexInputAttributeDescription> Attributes;
        const std::vector<VkVertexInputBindingDescription> Bindings;
    };

}

#endif // !VPSK_VERTEX_LAYOUT_HPP
