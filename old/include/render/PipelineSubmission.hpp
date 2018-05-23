#pragma once
#ifndef VPSK_BASE_PASS_HPP
#define VPSK_BASE_PASS_HPP
#include "ForwardDecl.hpp"
#include "resources/PipelineResource.hpp"
#include "util/Delegate.hpp"
#include <variant>
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <string>

#include "resource/PipelineCache.hpp"
#include "render/GraphicsPipeline.hpp"
#include "resource/PipelineLayout.hpp"
#include "resource/DescriptorPool.hpp"
#include "resource/DescriptorSet.hpp"
#include "resource/DescriptorSetLayout.hpp"
#include "util/Delegate.hpp"

namespace vpsk {

    class DescriptorResources;
    class ShaderGroup;
    class RenderGraph;

    struct compute_pipeline_t {
        ~compute_pipeline_t();
        VkPipeline Handle;
        VkComputePipelineCreateInfo Info;
        st::delegate_t<void(const VkCommandBuffer&)> DispatchFunc;
    };

    struct graphics_pipeline_t {
        ~graphics_pipeline_t();
        std::unique_ptr<vpr::GraphicsPipeline> Pipeline;
        std::unique_ptr<vpr::GraphicsPipelineInfo> Info;
    };

    using pipelines_variant_t = std::variant<
        compute_pipeline_t,
        graphics_pipeline_t
    >;

    class PipelineSubmission {
    public:

        PipelineSubmission(RenderGraph& rgraph, std::string name, size_t idx, VkPipelineStageFlags stages, 
            const std::string pipeline_options_json_path = "");
        ~PipelineSubmission();
        
        void AddShaders(const std::vector<std::string>& shader_paths, const std::vector<VkShaderStageFlagBits>& stages = {});
        void AddShaders(const std::vector<std::string>& shader_names, const std::vector<std::string>& shader_srcs, 
            const std::vector<VkShaderStageFlagBits>& stages);

        PipelineResource& SetDepthStencilInput(const std::string& name);
        PipelineResource& SetDepthStencilOutput(const std::string& name, image_info_t info);
        PipelineResource& AddAttachmentInput(const std::string& name);
        PipelineResource& AddHistoryInput(const std::string& name);
        PipelineResource& AddColorOutput(const std::string& name, image_info_t info, const std::string& input = "");
        PipelineResource& AddResolveOutput(const std::string& name, image_info_t info);
        PipelineResource& AddTextureInput(const std::string& name, image_info_t info);
        PipelineResource& AddStorageTextureOutput(const std::string& name, image_info_t info, const std::string& input = "");
        PipelineResource & AddStorageTextureRW(const std::string & name, image_info_t info);
        PipelineResource& AddUniformInput(const std::string& name);
        PipelineResource& AddStorageOutput(const std::string& name, buffer_info_t info, const std::string& input = "");
        PipelineResource& AddStorageReadOnlyInput(const std::string& name);

        const std::vector<PipelineResource*>& GetColorOutputs() const noexcept;
        const std::vector<PipelineResource*>& GetResolveOutputs() const noexcept;
        const std::vector<PipelineResource*>& GetColorInputs() const noexcept;
        const std::vector<PipelineResource*>& GetColorScaleInputs() const noexcept;
        const std::vector<PipelineResource*>& GetTextureInputs() const noexcept;
        const std::vector<PipelineResource*>& GetStorageTextureInputs() const noexcept;
        const std::vector<PipelineResource*>& GetStorageTextureOutputs() const noexcept;
        const std::vector<PipelineResource*>& GetAttachmentInputs() const noexcept;
        const std::vector<PipelineResource*>& GetHistoryInputs() const noexcept;
        const std::vector<PipelineResource*>& GetUniformInputs() const noexcept;
        const std::vector<PipelineResource*>& GetStorageOutputs() const noexcept;
        const std::vector<PipelineResource*>& GetStorageReadOnlyInputs() const noexcept;
        const std::vector<PipelineResource*>& GetStorageInputs() const noexcept;
        const PipelineResource* GetDepthStencilInput() const noexcept;
        const PipelineResource* GetDepthStencilOutput() const noexcept;

        void SetIdx(size_t idx);
        void SetPhysicalPassIdx(size_t idx);
        void SetStages(VkPipelineStageFlags _stages);
        void SetName(std::string name);

        const size_t& GetIdx() const noexcept;
        const size_t& GetPhysicalPassIdx() const noexcept;
        const VkPipelineStageFlags& GetStages() const noexcept;
        const std::string& GetName() const noexcept;
        RenderGraph& GetRenderGraph() noexcept;

        bool NeedRenderPass() const noexcept;
        bool GetClearColor(size_t idx, VkClearColorValue* value = nullptr) noexcept;
        bool GetClearDepth(VkClearDepthStencilValue* value = nullptr) const noexcept;
        void BuildSubmission(const VkCommandBuffer& cmd);

    private:

        RenderGraph& renderGraph;
        size_t idx;
        size_t physicalPass{ std::numeric_limits<size_t>::max() };
        VkPipelineStageFlags stages;
        std::string name;

        st::delegate_t<void(const VkCommandBuffer&)> buildPassCb;
        st::delegate_t<bool()> needPassCb;
        st::delegate_t<bool(VkClearDepthStencilValue*)> getClearDepthCb;
        st::delegate_t<bool(size_t, VkClearColorValue*)> getClearColorCb;
        
        std::vector<PipelineResource*> colorOutputs;
        std::vector<PipelineResource*> resolveOutputs;
        std::vector<PipelineResource*> colorInputs;
        std::vector<PipelineResource*> colorScaleInputs;
        std::vector<PipelineResource*> textureInputs;
        std::vector<PipelineResource*> storageTextureInputs;
        std::vector<PipelineResource*> storageTextureOutputs;
        std::vector<PipelineResource*> attachmentInputs;
        std::vector<PipelineResource*> historyInputs;
        std::vector<PipelineResource*> uniformInputs;
        std::vector<PipelineResource*> storageOutputs;
        std::vector<PipelineResource*> storageReadOnlyInputs;
        std::vector<PipelineResource*> storageInputs;

        PipelineResource* depthStencilInput{ nullptr };
        PipelineResource* depthStencilOutput{ nullptr };

        std::vector<size_t> usedSetIndices;
        std::unique_ptr<vpr::PipelineCache> cache;
        std::unique_ptr<vpr::PipelineLayout> layout;
        pipelines_variant_t pipeline;

    };
}

#endif //!VPSK_BASE_PASS_HPP