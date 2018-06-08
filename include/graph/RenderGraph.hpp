#pragma once
#ifndef VPSK_RENDERGRAPH_HPP
#define VPSK_RENDEGRAPH_HPP
#include "ForwardDecl.hpp"
#include <vulkan/vulkan.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include "core/ResourceUsage.hpp"
#include "PipelineResource.hpp"

namespace st {
    class ShaderPack;
    class ShaderResource;
    class ShaderGroup;
}

namespace vpsk {

    class PipelineSubmission;
    class BufferResourceCache;
    class ImageResourceCache;

    class RenderGraph {
        friend class PipelineSubmission;
    public:

        RenderGraph(const vpr::Device* dvc);
        ~RenderGraph();

        void AddShaderPack(const st::ShaderPack* pack);

        PipelineSubmission& AddSubmission(const std::string& name, VkPipelineStageFlags stages);
        PipelineResource& GetResource(const std::string& name);
        BufferResourceCache& GetBufferResourceCache();
        ImageResourceCache& GetImageResourceCache();
        void Bake();
        void Reset();

        void SetBackbufferSource(const std::string& name);
        void SetBackbufferDimensions(const resource_dimensions_t& dimensions);
        resource_dimensions_t GetResourceDimensions(const PipelineResource& rsrc);
        size_t NumSubmissions() const noexcept;

        const vpr::Device* GetDevice() const noexcept;

        static RenderGraph& GetGlobalGraph();
    private:

        void addShaderPackResources(const st::ShaderPack* pack);
        buffer_info_t createPipelineResourceBufferInfo(const st::ShaderResource* resource) const;
        image_info_t createPipelineResourceImageInfo(const st::ShaderResource* resource) const;
        void createPipelineResourcesFromPack(const std::unordered_map<std::string, std::vector<const st::ShaderResource*>>& resources);
        void addResourceUsagesToSubmission(PipelineSubmission& submissions, const std::vector<st::ResourceUsage>& usages);
        void addSingleGroup(const std::string& name, const st::ShaderGroup* group);
        void addSubmissionsFromPack(const st::ShaderPack* pack);

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
        std::unordered_map<std::string, size_t> submissionNameMap;
        std::vector<std::unique_ptr<PipelineSubmission>> pipelineSubmissions;
        std::unordered_map<std::string, size_t> resourceNameMap;
        std::vector<std::unique_ptr<PipelineResource>> pipelineResources;
        // string "name" is for the pack the resources belong to.
        std::unique_ptr<vpsk::BufferResourceCache> bufferResources;
        std::unique_ptr<vpsk::ImageResourceCache> imageResources;
        const vpr::Device* device;

        std::vector<std::unordered_set<size_t>> submissionDependencies;
        std::vector<std::unordered_set<size_t>> submissionMergeDependencies;
        std::vector<size_t> submissionStack;
    };

}
#endif // !VPSK_RENDERGRAPH_HPP
