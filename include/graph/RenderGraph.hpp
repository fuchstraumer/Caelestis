#pragma once
#ifndef VPSK_RENDERGRAPH_HPP
#define VPSK_RENDEGRAPH_HPP
#include "ForwardDecl.hpp"
#include <vulkan/vulkan.h>
#include <string>
#include <unordered_map>

namespace vpsk {

    class PipelineResource;
    class PipelineSubmission;
    class DescriptorResources;
    class ResourceDimensions;
    class ShaderBufferResources;

    class RenderGraph {
    public:

        RenderGraph(const vpr::Device* dvc);

        PipelineSubmission& AddSubmission(const std::string& name, VkPipelineStageFlags stages);
        PipelineResource& GetResource(const std::string& name);
        ShaderBufferResources& BufferResourceCache();

        void Bake();
        void Reset();

        void SetBackbufferSource(const std::string& name);
        void SetBackbufferDimensions(const ResourceDimensions& dimensions);

        const vpr::Device* GetDevice() const noexcept;
        DescriptorResources* GetDescriptorResources();

    private:

        struct pipeline_barrier_t {
            size_t Resource;
            VkImageLayout Layout{ VK_IMAGE_LAYOUT_UNDEFINED };
            VkAccessFlags AccessFlags{ VK_ACCESS_FLAG_BITS_MAX_ENUM };
            VkPipelineStageFlags Stages{ VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM };
            bool History{ false };
        };

        struct pass_barriers_t {
            std::vector<pipeline_barrier_t> InvalidateBarriers;
            std::vector<pipeline_barrier_t> FlushBarriers;
        };

        struct color_clear_request_t {
            PipelineSubmission* Submission;
            VkClearColorValue* Target;
            size_t Index;
        };

        struct depth_clear_request_t {
            PipelineSubmission* Submission;
            VkClearDepthStencilValue* Target;
        };

        struct scaled_clear_request_t {
            size_t Target;
            size_t Resource;
        };

        struct mipmap_request_t {
            size_t Resource;
        };

        std::string graphName;
        std::string backbufferSource;
        std::unordered_map<std::string, pass_barriers_t> passBarriers;
        std::unordered_map<std::string, vpr::Buffer*> backingBuffers;
        std::unordered_map<std::string, vpr::Image*> backingImages;
        std::unordered_map<std::string, size_t> resourceNameMap;
        std::vector<std::unique_ptr<PipelineResource>> resources;
        // string "name" is for the pack the resources belong to.
        std::unordered_map<std::string, std::unique_ptr<DescriptorResources>> descriptorResources;
        std::unordered_map<std::string, std::unique_ptr<vpr::DescriptorSet>> descriptorSets;
        std::unique_ptr<ShaderBufferResources> bufferResources;
        const vpr::Device* device;
    };

}
#endif // !VPSK_RENDERGRAPH_HPP
