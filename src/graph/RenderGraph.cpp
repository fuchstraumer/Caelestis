#include "graph/RenderGraph.hpp"
#include "graph/PipelineResource.hpp"
#include "graph/PipelineSubmission.hpp"
#include "resource/DescriptorSet.hpp"
#include "systems/BufferResourceCache.hpp"
#include "systems/ImageResourceCache.hpp"
#include "core/ShaderPack.hpp"
#include "core/ShaderResource.hpp"
#include "core/ShaderGroup.hpp"
#include "RendererCore.hpp"
namespace vpsk {

    static constexpr VkPipelineStageFlags ShaderStagesToPipelineStages(const VkShaderStageFlags& flags) {
        VkPipelineStageFlags result = 0;
        if (flags & VK_SHADER_STAGE_VERTEX_BIT) {
            result |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        }
        if (flags & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) {
            result |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
        }
        if (flags & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) {
            result |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
        }
        if (flags & VK_SHADER_STAGE_GEOMETRY_BIT) {
            result |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
        }
        if (flags & VK_SHADER_STAGE_FRAGMENT_BIT) {
            result |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        if (flags & VK_SHADER_STAGE_COMPUTE_BIT) {
            result |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        }
        return result;
    }

    static constexpr VkBufferUsageFlags buffer_usage_from_descriptor_type(const VkDescriptorType type) noexcept {
        // I'm using transfer_dst as the base, as I'd prefer to keep all of these buffers on the device
        // as device-local memory when possible
        if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
            return VkBufferUsageFlags{ VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT };
        }
        else if (type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER || type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
            return VkBufferUsageFlags{ VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT };
        }
        else if (type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) {
            return VkBufferUsageFlags{ VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT };
        }
        else if (type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) {
            return VkBufferUsageFlags{ VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT };
        }
        else {
            return VkBufferUsageFlags{ VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM };
        }
    }

    static constexpr VkImageUsageFlags image_usage_from_descriptor_type(const VkDescriptorType type) noexcept {
        if (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER || type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) {
            return VkImageUsageFlags{ VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT };
        }
        else if (type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
            return VkImageUsageFlags{ VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT };
        }
        else if (type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
            return VkImageUsageFlags{ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT };
        }
        else {
            return VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
        }
    }

    static constexpr bool is_buffer_type(const VkDescriptorType& type) {
        if (type == VK_DESCRIPTOR_TYPE_SAMPLER ||
            type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
            type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
            type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
            type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
            return false;
        }
        else {
            return true;
        }
    }

    RenderGraph::RenderGraph(const vpr::Device* dvc) : device(dvc) {
        bufferResources = std::make_unique<vpsk::BufferResourceCache>(device);
        imageResources = std::make_unique<vpsk::ImageResourceCache>(device);
    }

    RenderGraph::~RenderGraph() {}

    void RenderGraph::AddShaderPack(const st::ShaderPack * pack) {
        addShaderPackResources(pack);
    }

    PipelineSubmission& RenderGraph::AddSubmission(const std::string& name, VkPipelineStageFlags stages) {
        auto iter = submissionNameMap.find(name);
        if (iter != std::end(submissionNameMap)) {
            return *pipelineSubmissions[iter->second];
        }
        else {
            size_t idx = pipelineSubmissions.size();
            pipelineSubmissions.emplace_back(std::make_unique<PipelineSubmission>(*this, name, idx, stages));
            submissionNameMap[name] = idx;
            return *pipelineSubmissions.back();
        }
    }

    PipelineResource& RenderGraph::GetResource(const std::string& name) {
        auto iter = resourceNameMap.find(name);
        if (iter != resourceNameMap.cend()) {
            const size_t& idx = iter->second;
            return *pipelineResources[idx];
        }
        else {
            const size_t idx = pipelineResources.size();
            pipelineResources.emplace_back(std::make_unique<PipelineResource>(name, idx));
            resourceNameMap[name] = idx;
            return *pipelineResources.back();
        }
    }

    void RenderGraph::addShaderPackResources(const st::ShaderPack* pack) {

        std::vector<std::string> resource_group_names;
        {
            st::dll_retrieved_strings_t names = pack->GetResourceGroupNames();
            for (size_t i = 0; i < names.NumStrings; ++i) {
                resource_group_names.emplace_back(names[i]);
            }
        }

        std::unordered_map<std::string, std::vector<const st::ShaderResource*>> resources;
        
        for (auto& name : resource_group_names) {
            size_t num_rsrc = 0;
            pack->GetResourceGroupPointers(name.c_str(), &num_rsrc, nullptr);
            std::vector<const st::ShaderResource*> group(num_rsrc);
            pack->GetResourceGroupPointers(name.c_str(), &num_rsrc, group.data());
            resources.emplace(name.c_str(), group);
        }

        for (auto& group : resources) {
            imageResources->AddResources(group.second);
            bufferResources->AddResources(group.second);
        }
        
        createPipelineResourcesFromPack(resources);
    }

    BufferResourceCache& RenderGraph::GetBufferResourceCache() {
        return *bufferResources;
    }

    ImageResourceCache& RenderGraph::GetImageResourceCache() {
        return *imageResources;
    }

    void RenderGraph::Bake() {
        for (const auto& submission : pipelineSubmissions) {
            submission->ValidateSubmission();
        }


    }

    const vpr::Device* RenderGraph::GetDevice() const noexcept {
        return device;
    }

    RenderGraph& RenderGraph::GetGlobalGraph() {
        static RenderGraph graph(RendererCore::GetRenderer().Device());
        return graph;
    }

    buffer_info_t RenderGraph::createPipelineResourceBufferInfo(const st::ShaderResource* resource) const {
        return buffer_info_t{ resource->MemoryRequired(), buffer_usage_from_descriptor_type(resource->DescriptorType()) };
    }

    image_info_t RenderGraph::createPipelineResourceImageInfo(const st::ShaderResource* resource) const {
        image_info_t result;
        if (resource->DescriptorType() == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
            result.SizeClass = image_info_t::size_class::SwapchainRelative;
        }
        result.Usage = image_usage_from_descriptor_type(resource->DescriptorType());
        result.Format = resource->Format();
        const VkImageCreateInfo& image_info = resource->ImageInfo();
        result.ArrayLayers = image_info.arrayLayers;
        result.MipLevels = image_info.mipLevels;
        result.Samples = image_info.samples;
        if (resource->DescriptorType() == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
            result.Anisotropy = resource->SamplerInfo().maxAnisotropy;
        }
        return result;
    }

    void RenderGraph::createPipelineResourcesFromPack(const std::unordered_map<std::string, std::vector<const st::ShaderResource*>>& resources) {
        for (const auto& rsrc_group : resources) {
            for (const auto& st_rsrc : rsrc_group.second) {
                auto& resource = GetResource(st_rsrc->Name());
                resource.SetDescriptorType(st_rsrc->DescriptorType());
                if (st_rsrc->DescriptorType() == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
                    resource.SetStorage(true);
                }
                resource.SetParentSetName(st_rsrc->ParentGroupName());
                if (is_buffer_type(st_rsrc->DescriptorType())) {
                    resource.SetInfo(createPipelineResourceBufferInfo(st_rsrc));
                }
                else {
                    resource.SetInfo(createPipelineResourceImageInfo(st_rsrc));
                }

            }
        }
    }

    void RenderGraph::addResourceUsagesToSubmission(PipelineSubmission & submissions, const std::vector<st::ResourceUsage>& usages) {
        for (const auto& usage : usages) {

        }
    }

    void RenderGraph::addSingleGroup(const std::string & name, const st::ShaderGroup * group) {
        PipelineSubmission& submission = AddSubmission(name, ShaderStagesToPipelineStages(group->Stages()));
        
        const size_t num_sets_in_submissions = group->GetNumSetsRequired();
        for (size_t i = 0; i < num_sets_in_submissions; ++i) {
            size_t num_rsrcs = 0;
            group->GetResourceUsages(i, &num_rsrcs, nullptr);
            std::vector<st::ResourceUsage> usages(num_rsrcs);
            group->GetResourceUsages(i, &num_rsrcs, usages.data());
            addResourceUsagesToSubmission(submission, usages);
        }
        
    }

    void RenderGraph::addSubmissionsFromPack(const st::ShaderPack* pack) {
        std::vector<std::string> pack_names;
        {
            auto pack_retrieved = pack->GetShaderGroupNames();
            for (size_t i = 0; i < pack_retrieved.NumStrings; ++i) {
                pack_names.emplace_back(pack_retrieved[i]);
            }
        }

        for (auto&& name : pack_names) {
            addSingleGroup(name, pack->GetShaderGroup(name.c_str()));
        }

    }

}
