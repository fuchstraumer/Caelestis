#pragma once
#ifndef VPSK_COMMON_STRUCTURES_HPP
#define VPSK_COMMON_STRUCTURES_HPP
#include <cstdint>


namespace vpsk {

    struct model_part_t {
        uint32_t firstIndex{ 0 };
        uint32_t indexCount{ 0 };
        int32_t vertexOffset{ 0 };
        size_t materialIndex{ 0 };

        bool operator==(const model_part_t& other) const noexcept {
            return (indexCount == other.indexCount) && (materialIndex == other.materialIndex) &&
                (firstIndex == other.firstIndex);
        }

        bool operator<(const model_part_t& other) const noexcept {
            if (materialIndex == other.materialIndex) {
                // should help sort by index data locality among objects sharing a material
                return firstIndex < other.firstIndex;
            }
            else {
                return materialIndex < other.materialIndex;
            }
        }
        
    };


    struct vertex_layout_t {
        vertex_layout_t(const std::vector<VkVertexInputAttributeDescription>& attrs, const std::vector<VkVertexInputBindingDescription>& bindings) :
            Attributes(attrs), Bindings(Bindings) {}
        vertex_layout_t(const std::vector<VkVertexInputAttributeDescription>& attrs) : Attributes(attrs), Bindings(getBindings(attrs)) {}
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

#endif //!VPSK_COMMON_STRUCTURES_HPP