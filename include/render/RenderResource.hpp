#pragma once
#ifndef VPSK_RENDER_RESOURCE_HPP
#define VPSK_RENDER_RESOURCE_HPP
#include "ForwardDecl.hpp"
#include <vulkan/vulkan.h>
#include <variant>
#include <unordered_set>
namespace vpsk {

    class RenderResource {
    public:
        RenderResource(const size_t& idx, const std::string& _name = std::string());
        virtual ~RenderResource() = default;

        const std::unordered_set<uint8_t>& ReadInPasses() const noexcept;
        const std::unordered_set<uint8_t>& WrittenInPasses() const noexcept;
        const std::string& Name() const noexcept;
        const size_t& Index() const noexcept;
        const size_t& PhysicalIndex() const noexcept;
        const VkPipelineStageFlags& UsedInStages() const noexcept;

        void WrittenInPass(const uint8_t& subpass_idx) noexcept;
        void ReadInPass(const uint8_t& subpass_idx) noexcept;
        void SetName(const std::string& _name) noexcept;
        void SetIndex(const size_t& idx) noexcept;
        void SetPhysicalIndex(const size_t& phys_idx) noexcept;
        void SetUsedInStages(const VkPipelineStageFlags& flags) noexcept;
        void AddStage(const VkPipelineStageFlags& flags) noexcept;

    private:
        VkPipelineStageFlags usedInStages;
        std::string name;
        size_t idx;
        size_t physicalIdx;
        // Log when this is read or written, to build barriers and dependency graph
        std::unordered_set<uint8_t> readInPasses;
        std::unordered_set<uint8_t> writtenInPasses;
    };

    /** The buffer resource represents a buffer object that will be used by a renderpass
     *  at some point. It mainly serves as utility to help us build the rendergraph, and
     *  includes a comparison operator that checks to see if the backing resource is ac-
     *  tually the same. If so, then we need to consider this object when building the
     *  various memory barriers and safety features.
     *  \ingroup Rendering
     */

    class BufferResource : public RenderResource {
    public:

    private:

        vpr::Buffer* object;
    };

    class ImageResource : public RenderResource {

    };

    /**To avoid tracking of resource subtype via enum, and to let us store things 
     * all in one container, we use a variant to hold only one of the above objects
     * + let us keep type info that we can check against as needed.
     * \ingroup Rendering
     */
    using subpass_resource_t = std::variant<BufferResource, ImageResource>;


}

#endif //!VPSK_RENDER_RESOURCE_HPP