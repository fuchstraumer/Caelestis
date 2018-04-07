#pragma once
#ifndef VPSK_PIPELINE_RESOURCE_HPP
#define VPSK_PIPELINE_RESOURCE_HPP
#include "ForwardDecl.hpp"
#include "vulkan/vulkan.h"
#include <string>
#include <variant>
#include <unordered_set>

namespace vpsk {
    
    struct buffer_info_t {
        VkDeviceSize Size{ 0 };
        VkBufferUsageFlags Usage{ VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM };
        bool operator==(const buffer_info_t& other) const noexcept;
    };

    struct image_info_t {
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
        VkImageUsageFlags Usage{ VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM };
        VkFormat Format{ VK_FORMAT_UNDEFINED };
        bool operator==(const image_info_t& other) const noexcept;
    };

    using resource_info_variant_t = std::variant<
        buffer_info_t,
        image_info_t
    >;

    struct ResourceDimensions {
        VkFormat Format{ VK_FORMAT_UNDEFINED };
        buffer_info_t BufferInfo;
        uint32_t Width{ 0 };
        uint32_t Height{ 0 };
        uint32_t Depth{ 1 };
        uint32_t Layers{ 1 };
        uint32_t Levels{ 1 };
        uint32_t Samples{ 1 };
        bool Transient{ false };
        bool Persistent{ true };
        bool Storage{ false };
        VkPipelineStageFlags UsedStages{ VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM };
    };

    class PipelineResource {
    public:

        PipelineResource(std::string name, size_t idx);

        bool operator==(const PipelineResource& other) const noexcept;

        bool IsBuffer() const noexcept;
        bool IsImage() const noexcept;

        void WrittenInPass(uint16_t idx);
        void ReadInPass(uint16_t idx);

        void SetIdx(size_t idx);
        void SetParentSetIdx(size_t idx);
        void SetName(std::string _name);
        void SetDescriptorType(VkDescriptorType type);
        void AddUsedPipelineStages(VkPipelineStageFlags stages);
        void SetInfo(resource_info_variant_t _info);

        const size_t& GetIdx() const noexcept;
        const size_t& GetParentSetIdx() const noexcept;
        const VkDescriptorType& GetDescriptorType() const noexcept;
        const std::string& GetName() const noexcept;
        VkPipelineStageFlags GetUsedPipelineStages() const noexcept;
        const std::unordered_set<uint16_t>& GetPassesReadIn() const noexcept;
        const std::unordered_set<uint16_t>& GetPassesWrittenIn() const noexcept;
        const resource_info_variant_t& GetInfo() const noexcept;

    private:

        size_t idx;
        // We don't care about the unique binding index of a resource relative to a set: just what
        // set it belongs to, as that helps us group things in a more important manner
        size_t parentSetIdx{ std::numeric_limits<size_t>::max() };
        // Usually the name read from the shader
        std::string name{ };
        // Extracted from the shader
        VkDescriptorType intendedType{ VK_DESCRIPTOR_TYPE_MAX_ENUM };
        // Stages of the pipeline this item is used in
        VkPipelineStageFlags usedStages{ VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM };

        resource_info_variant_t info;

        std::unordered_set<uint16_t> readInPasses;
        std::unordered_set<uint16_t> writtenInPasses;
    };

}

#endif // !VPSK_PIPELINE_RESOURCE_HPP
