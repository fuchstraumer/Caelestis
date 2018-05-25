#include "graph/PipelineSubmission.hpp"
#include "graph/RenderGraph.hpp"
namespace vpsk {

    PipelineSubmission::PipelineSubmission(RenderGraph& rgraph, std::string _name, size_t _idx, VkPipelineStageFlags _stages) 
        : graph(rgraph), name(std::move(_name)), idx(std::move(_idx)), stages(std::move(_stages)) {}

    PipelineSubmission::~PipelineSubmission() {}

    PipelineResource& PipelineSubmission::SetDepthStencilInput(const std::string& name) {
        auto& resource = graph.GetResource(name);
        resource.AddUsedPipelineStages(stages);
        resource.ReadInPass(idx);
        depthStencilInput = &resource;
        return resource;
    }

    PipelineResource& PipelineSubmission::SetDepthStencilOutput(const std::string& name, image_info_t info) {
        auto& resource = graph.GetResource(name);
        resource.AddUsedPipelineStages(stages);
        resource.WrittenInPass(idx);
        resource.SetInfo(info);
        depthStencilOutput = &resource;
        return resource;
    }

    PipelineResource& PipelineSubmission::AddAttachmentInput(const std::string& name) {
        auto& resource = graph.GetResource(name);
        resource.AddUsedPipelineStages(stages);
        resource.ReadInPass(idx);
        attachmentInputs.emplace_back(&resource);
        return resource;
    }

    PipelineResource& PipelineSubmission::AddHistoryInput(const std::string& name) {
        auto& resource = graph.GetResource(name);
        resource.AddUsedPipelineStages(stages);
        historyInputs.emplace_back(&resource);
        return resource;
    }

    PipelineResource& PipelineSubmission::AddColorOutput(const std::string& name, image_info_t info, const std::string& input) {
        auto& resource = graph.GetResource(name);
        resource.AddUsedPipelineStages(stages);
        resource.WrittenInPass(idx);
        resource.SetInfo(info);
        colorOutputs.emplace_back(&resource);
        if (!input.empty()) {
            auto& input_resource = graph.GetResource(input);
            input_resource.ReadInPass(idx);
            colorInputs.emplace_back(&input_resource);
            colorScaleInputs.emplace_back(&input_resource);
        }
        else {
            colorInputs.emplace_back(nullptr);
            colorScaleInputs.emplace_back(nullptr);
        }
        return resource;
    }

    PipelineResource& PipelineSubmission::AddResolveOutput(const std::string& name, image_info_t info) {
        auto& resource = graph.GetResource(name);
        resource.AddUsedPipelineStages(stages);
        resource.WrittenInPass(idx);
        resource.SetInfo(info);
        resolveOutputs.emplace_back(&resource);
        return resource;
    }

    PipelineResource& PipelineSubmission::AddTextureInput(const std::string& name, image_info_t info) {
        auto& resource = graph.GetResource(name);
        resource.AddUsedPipelineStages(stages);
        resource.ReadInPass(idx);
        textureInputs.emplace_back(&resource);
        return resource;
    }

    PipelineResource& PipelineSubmission::AddStorageTextureOutput(const std::string& name, image_info_t info, const std::string& input) {
        auto& resource = graph.GetResource(name);
        resource.AddUsedPipelineStages(stages);
        resource.WrittenInPass(idx);
        resource.SetInfo(info);
        resource.SetStorage(true);
        storageTextureOutputs.emplace_back(&resource);
        if (!input.empty()) {
            auto& input_resource = graph.GetResource(input);
            input_resource.ReadInPass(idx);
            storageTextureInputs.emplace_back(&input_resource);
        }
        else {
            storageTextureInputs.emplace_back(nullptr);
        }
        return resource;
    }

    PipelineResource& PipelineSubmission::AddStorageTextureRW(const std::string& name, image_info_t info) {
        auto& resource = graph.GetResource(name);
        resource.AddUsedPipelineStages(stages);
        resource.ReadInPass(idx);
        resource.WrittenInPass(idx);
        resource.SetStorage(true);
        storageTextureOutputs.emplace_back(&resource);
        storageTextureInputs.emplace_back(&resource);
        return resource;
    }

    PipelineResource& PipelineSubmission::AddUniformInput(const std::string& name) {
        auto& resource = graph.GetResource(name);
        resource.AddUsedPipelineStages(stages);
        resource.ReadInPass(idx);
        uniformInputs.emplace_back(&resource);
        return resource;
    }

    PipelineResource& PipelineSubmission::AddStorageOutput(const std::string& name, buffer_info_t info, const std::string& input) {
        auto& resource = graph.GetResource(name);
        resource.AddUsedPipelineStages(stages);
        resource.WrittenInPass(idx);
        resource.SetInfo(info);
        resource.SetStorage(true);
        storageOutputs.emplace_back(&resource);
        if (!input.empty()) {
            auto& input_resource = graph.GetResource(input);
            input_resource.ReadInPass(idx);
            storageInputs.emplace_back(&input_resource);
        }
        else {
            storageInputs.emplace_back(nullptr);
        }
        return resource;
    }

    PipelineResource& PipelineSubmission::AddStorageReadOnlyInput(const std::string& name) {
        auto& resource = graph.GetResource(name);
        resource.AddUsedPipelineStages(stages);
        resource.ReadInPass(idx);
        storageReadOnlyInputs.emplace_back(&resource);
        return resource;
    }

    const std::vector<PipelineResource*>& PipelineSubmission::GetColorOutputs() const noexcept {
        return colorOutputs;
    }

    const std::vector<PipelineResource*>& PipelineSubmission::GetResolveOutputs() const noexcept {
        return resolveOutputs;
    }

    const std::vector<PipelineResource*>& PipelineSubmission::GetColorInputs() const noexcept {
        return colorInputs;
    }

    const std::vector<PipelineResource*>& PipelineSubmission::GetColorScaleInputs() const noexcept {
        return colorScaleInputs;
    }

    const std::vector<PipelineResource*>& PipelineSubmission::GetTextureInputs() const noexcept {
        return textureInputs;
    }

    const std::vector<PipelineResource*>& PipelineSubmission::GetStorageTextureInputs() const noexcept {
        return storageTextureInputs;
    }

    const std::vector<PipelineResource*>& PipelineSubmission::GetStorageTextureOutputs() const noexcept {
        return storageTextureOutputs;
    }

    const std::vector<PipelineResource*>& PipelineSubmission::GetAttachmentInputs() const noexcept {
        return attachmentInputs;
    }

    const std::vector<PipelineResource*>& PipelineSubmission::GetHistoryInputs() const noexcept {
        return historyInputs;
    }

    const std::vector<PipelineResource*>& PipelineSubmission::GetUniformInputs() const noexcept {
        return uniformInputs;
    }

    const std::vector<PipelineResource*>& PipelineSubmission::GetStorageOutputs() const noexcept {
        return storageOutputs;
    }

    const std::vector<PipelineResource*>& PipelineSubmission::GetStorageReadOnlyInputs() const noexcept {
        return storageReadOnlyInputs;
    }

    const std::vector<PipelineResource*>& PipelineSubmission::GetStorageInputs() const noexcept {
        return storageInputs;
    }

    const PipelineResource * PipelineSubmission::GetDepthStencilInput() const noexcept {
        return depthStencilInput;
    }

    const PipelineResource * PipelineSubmission::GetDepthStencilOutput() const noexcept {
        return depthStencilOutput;
    }

    void PipelineSubmission::SetIdx(size_t _idx) {
        idx = std::move(_idx);
    }

    void PipelineSubmission::SetPhysicalPassIdx(size_t idx) {
        physicalPassIdx = std::move(idx);
    }

    void PipelineSubmission::SetStages(VkPipelineStageFlags _stages) {
        stages = std::move(_stages);
    }

    void PipelineSubmission::SetName(std::string _name) {
        name = std::move(_name);
    }

    const size_t& PipelineSubmission::GetIdx() const noexcept {
        return idx;
    }

    const size_t& PipelineSubmission::GetPhysicalPassIdx() const noexcept {
        return physicalPassIdx;
    }

    const VkPipelineStageFlags& PipelineSubmission::GetStages() const noexcept {
        return stages;
    }

    const std::string& PipelineSubmission::GetName() const noexcept {
        return name;
    }

    bool PipelineSubmission::NeedRenderPass() const noexcept {
        if (!needPassCb) {
            return true;
        }
        else {
            return needPassCb();
        }
    }

    bool PipelineSubmission::GetClearColor(size_t idx, VkClearColorValue* value) noexcept {
        if (!getClearColorValueCb) {
            return false;
        }
        else {
            return getClearColorValueCb(idx, value);
        }
    }

    bool PipelineSubmission::GetClearDepth(VkClearDepthStencilValue* value) const noexcept {
        if (!getClearDepthValueCb) {
            return false;
        }
        else {
            return getClearDepthValueCb(value);
        }
    }

    void PipelineSubmission::RecordCommands(VkCommandBuffer cmd) {
        recordSubmissionCb(cmd);
    }

}
