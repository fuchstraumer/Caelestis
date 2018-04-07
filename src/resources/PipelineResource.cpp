#include "resources/PipelineResource.hpp"
namespace vpsk {


    PipelineResource::PipelineResource(std::string _name, resource_info_variant_t _info) : name(std::move(_name)),
        info(std::move(_info)) {}

    bool PipelineResource::IsBuffer() const noexcept {
        return std::holds_alternative<buffer_info_t>(info);
    }

    bool PipelineResource::IsImage() const noexcept {
        return std::holds_alternative<image_info_t>(info);
    }

    void PipelineResource::WrittenInPass(uint16_t idx) {
        writtenInPasses.emplace(std::move(idx));
    }

    void PipelineResource::ReadInPass(uint16_t idx) {
        readInPasses.emplace(std::move(idx));
    }

    void PipelineResource::SetIdx(size_t _idx) {
        idx = std::move(_idx);
    }

    void PipelineResource::SetParentSetIdx(size_t idx) {
        parentSetIdx = std::move(idx);
    }

    void PipelineResource::SetName(std::string _name) {
        name = std::move(_name);
    }

    void PipelineResource::SetDescriptorType(VkDescriptorType type) {
        intendedType = std::move(type);
    }

    void PipelineResource::AddUsedPipelineStages(VkPipelineStageFlags stages) {
        usedStages |= stages;
    }

    const size_t& PipelineResource::GetIdx() const noexcept {
        return idx;
    }

    const size_t& PipelineResource::GetParentSetIdx() const noexcept {
        return parentSetIdx;
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

    const std::unordered_set<uint16_t>& PipelineResource::GetPassesReadIn() const noexcept {
        return readInPasses;
    }

    const std::unordered_set<uint16_t>& PipelineResource::GetPassesWrittenIn() const noexcept {
        return writtenInPasses;
    }
}
