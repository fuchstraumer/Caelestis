#include "graph/PipelineResource.hpp"

namespace vpsk {

    PipelineResource::PipelineResource(std::string _name, size_t physical_idx) : name(std::move(_name)), 
        idx(std::move(physical_idx))  {}

    PipelineResource::~PipelineResource() {}

    bool PipelineResource::IsBuffer() const {
        return std::holds_alternative<buffer_info_t>(info);
    }

    bool PipelineResource::IsImage() const {
        return std::holds_alternative<image_info_t>(info);
    }

    bool PipelineResource::IsStorage() const noexcept {
        return storage;
    }

    bool PipelineResource::IsTransient() const noexcept {
        return transient;
    }

    void PipelineResource::WrittenBySubmission(size_t idx) {
        writtenInPasses.emplace(std::move(idx));
    }

    void PipelineResource::ReadBySubmission(size_t idx) {
        readInPasses.emplace(std::move(idx));
    }

    void PipelineResource::SetIdx(size_t _idx) {
        idx = std::move(_idx);
    }

    void PipelineResource::SetParentSetName(std::string _name) {
        parentSetName = std::move(_name);
    }

    void PipelineResource::SetName(std::string _name) {
        name = std::move(_name);
    }

    void PipelineResource::SetDescriptorType(VkDescriptorType type) {
        intendedType = std::move(type);
    }

    void PipelineResource::SetUsedPipelineStages(VkPipelineStageFlags stages) {
        usedStages = std::move(stages);
    }

    void PipelineResource::AddUsedPipelineStages(VkPipelineStageFlags stages) {
        usedStages |= stages;
    }

    void PipelineResource::SetInfo(resource_info_variant_t _info) {
        info = std::move(_info);
    }

    void PipelineResource::SetStorage(const bool& _storage) {
        storage = _storage;
    }

    void PipelineResource::SetTransient(const bool& _transient) {
        transient = _transient;
    }

    const size_t& PipelineResource::GetIdx() const noexcept {
        return idx;
    }

    const std::string& PipelineResource::GetParentSetName() const noexcept {
        return parentSetName;
    }

    const VkDescriptorType& PipelineResource::GetDescriptorType() const noexcept {
        return intendedType;
    }

    const std::string& PipelineResource::GetName() const noexcept {
        return name;
    }

    VkPipelineStageFlags PipelineResource::GetUsedPipelineStages() const noexcept {
        return usedStages;
    }

    const std::unordered_set<size_t>& PipelineResource::GetPassesWrittenIn() const noexcept {
        return writtenInPasses;
    }

    const std::unordered_set<size_t>& PipelineResource::GetPassesReadIn() const noexcept {
        return readInPasses;
    }

    const resource_info_variant_t& PipelineResource::GetInfo() const noexcept {
        return info;
    }

    bool buffer_info_t::operator==(const buffer_info_t& other) const noexcept {
        return (Size == other.Size) && (Usage == other.Usage);
    }

    bool image_info_t::operator==(const image_info_t& other) const noexcept {
        return (SizeClass == other.SizeClass) && (SizeX == other.SizeX) && (SizeY == other.SizeY) &&
            (Samples == other.Samples) && (MipLevels == other.MipLevels) && (ArrayLayers == other.ArrayLayers) &&
            (Anisotropy == other.Anisotropy) && (SizeRelativeName == other.SizeRelativeName) && (Usage == other.Usage) && 
            (Format == other.Format);
    }

}
