#pragma once
#ifndef VPSK_VERTEX_LAYOUT_HPP
#define VPSK_VERTEX_LAYOUT_HPP
#include <vulkan/vulkan.h>
#include <vector>

namespace vpsk {

    struct VertexLayout {
        VertexLayout(const std::vector<VkVertexInputAttributeDescription>& attrs, const std::vector<VkVertexInputBindingDescription>& bindings) :
            Attributes(attrs), Bindings(Bindings) {}
        VertexLayout(const std::vector<VkVertexInputAttributeDescription>& attrs) : Attributes(attrs), Bindings(getBindings(attrs)) {}
        const std::vector<VkVertexInputAttributeDescription> Attributes;
        const std::vector<VkVertexInputBindingDescription> Bindings;
    private:
        static std::vector<VkVertexInputBindingDescription> getBindings(const std::vector<VkVertexInputAttributeDescription>& attrs) {
            std::vector<VkVertexInputBindingDescription> results(2); // Default is to split into two buffers
            results[0].binding = 0;
            results[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            results[0].stride = 3 * sizeof(float);
            results[1].binding = 1;
            results[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            results[1].stride = 8 * sizeof(float);
            return results;
        }
    };

}

#endif // !VPSK_VERTEX_LAYOUT_HPP
