#include "render/RenderResource.hpp"
#include "resource/Buffer.hpp"
namespace vpsk {

    RenderResource::RenderResource(const size_t & _idx, const std::string & _name) : idx(_idx), name(_name) {}

    const std::unordered_set<uint16_t>& RenderResource::ReadInPasses() const noexcept {
        return readInPasses;
    }

    const std::unordered_set<uint16_t>& RenderResource::WrittenInPasses() const noexcept {
        return writtenInPasses;
    }

    const std::string& RenderResource::Name() const noexcept {
        return name;
    }

    const size_t& RenderResource::Index() const noexcept {
        return idx;
    }

    const size_t & RenderResource::PhysicalIndex() const noexcept {
        return physicalIdx;
    }

    const VkPipelineStageFlags & RenderResource::UsedInStages() const noexcept {
        return usedInStages;
    }

    void RenderResource::WrittenInPass(const uint16_t & subpass_idx) noexcept {
        writtenInPasses.insert(subpass_idx);
    }

    void RenderResource::ReadInPass(const uint16_t& subpass_idx) noexcept {
        readInPasses.insert(subpass_idx);
    }

    void RenderResource::SetName(const std::string & _name) noexcept {
        name = _name;
    }

    void RenderResource::SetIndex(const size_t & _idx) noexcept {
        idx = _idx;
    }

    void RenderResource::SetPhysicalIndex(const size_t & phys_idx) noexcept {
        physicalIdx = phys_idx;
    }

    void RenderResource::SetUsedInStages(const VkPipelineStageFlags & flags) noexcept {
        usedInStages = flags;
    }

    void RenderResource::AddStage(const VkPipelineStageFlags & flags) noexcept {
        usedInStages |= flags;
    }

    VkBufferUsageFlags BufferResource::Usage() const noexcept {
        return object->Usage();
    }

}