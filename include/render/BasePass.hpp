#pragma once
#ifndef VPSK_BASE_PASS_HPP
#define VPSK_BASE_PASS_HPP
#include "ForwardDecl.hpp"
#include <variant>
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <string>

namespace vpsk {

    class DescriptorResources;
    class ShaderGroup;

    struct compute_pipeline_t {
        VkPipeline Handle;
        VkComputePipelineCreateInfo Info;
    };

    struct graphics_pipeline_t {
        std::unique_ptr<vpr::GraphicsPipeline> Pipeline;
        std::unique_ptr<vpr::GraphicsPipelineInfo> Info;
    };

    using pipelines_variant_t = std::variant<
        compute_pipeline_t,
        graphics_pipeline_t
    >;

    class BasePass {
    public:

        BasePass(const vpr::Device* dvc, DescriptorResources* rsrcs);
        
        void AddShaders(const std::vector<std::string>& shader_paths, const std::vector<VkShaderStageFlagBits>& stages = {});
        void AddShaders(const std::vector<std::string>& shader_names, const std::vector<std::string>& shader_srcs, const std::vector<VkShaderStageFlagBits>& stages);

    private:

        const vpr::Device* device;
        DescriptorResources* resources;

        std::vector<size_t> usedSetIndices;
        std::unique_ptr<vpr::PipelineCache> cache;
        std::unique_ptr<vpr::PipelineLayout> layout;
        std::unique_ptr<ShaderGroup> shaders;
        pipelines_variant_t pipeline;

    };
}

#endif //!VPSK_BASE_PASS_HPP