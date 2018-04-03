#pragma once
#ifndef VPSK_PIPELINE_RESOURCE_HPP
#define VPSK_PIPELINE_RESOURCE_HPP
#include "ForwardDecl.hpp"
#include "vulkan/vulkan.h"
#include <string>
#include <variant>
#include <unordered_set>

namespace vpsk {
    
    struct BufferInfo {
        VkDeviceSize Size{ 0 };
        VkBufferUsageFlags Usage{ 0 };
        VkDeviceSize ViewOffset{ 0 };
    };

    struct ImageInfo {
        enum class size_class {
            Absolute,
            SwapchainRelative,
            InputRelative
        } SizeClass{ size_class::SwapchainRelative };
        float SizeX{ 1.0f };
        float SizeY{ 1.0f };
        uint32_t Samples{ 1 };
        uint32_t MIP_Levels{ 1 };
        uint32_t Layers{ 1 };
        float Anisotropy{ 1.0f };
        std::string SizeRelativeName{};
        VkImageUsageFlags Usage{ 0 };
    };


    class PipelineResource {
    public:

        PipelineResource(std::string name, const std::variant<BufferInfo, ImageInfo>& _info);

    private:

        // Usually the name read from the shader
        std::string name{ };
        // Extracted from the shader
        VkDescriptorType intendedType{ VK_DESCRIPTOR_TYPE_MAX_ENUM };
        // Says we don't actually need backing memory
        bool transient{ false };
        // Persists beyond the lifetime of the renderpass
        bool persistent{ true };
        // Used as storage, i.e a generalized format thats more compatible 
        bool storage{ false };
        // Stages of the pipeline this item is used in
        VkPipelineStageFlags stages{ 0 };

        std::variant<BufferInfo, ImageInfo> info;
        vpr::Buffer* backingBuffer{ nullptr };
        vpr::Image* backingImage{ nullptr };

        std::unordered_set<uint16_t> readInPasses;
        std::unordered_set<uint16_t> writtenInPasses;
    };

}

#endif // !VPSK_PIPELINE_RESOURCE_HPP
