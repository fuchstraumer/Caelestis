#include "graph/RenderGraph.hpp"
#include "graph/PipelineResource.hpp"
#include "resource/DescriptorSet.hpp"
#include "systems/BufferResourceCache.hpp"
#include "systems/ImageResourceCache.hpp"
#include "core/ShaderPack.hpp"
#include "core/ShaderResource.hpp"

namespace vpsk {

    RenderGraph::RenderGraph(const vpr::Device * dvc) : device(dvc) {
        bufferResources = std::make_unique<vpsk::BufferResourceCache>(device);
        imageResources = std::make_unique<vpsk::ImageResourceCache>(device);
    }

    RenderGraph::~RenderGraph() {}

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

    void RenderGraph::AddShaderPackResources(const st::ShaderPack* pack) {

        std::vector<std::string> resource_group_names;
        {
            st::dll_retrieved_strings_t names = pack->GetResourceGroupNames();
            for (size_t i = 0; i < names.NumStrings; ++i) {
                resource_group_names.emplace_back(names.Strings[i]);
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
        
    }

    BufferResourceCache& RenderGraph::GetBufferResourceCache() {
        return *bufferResources;
    }

    ImageResourceCache& RenderGraph::GetImageResourceCache() {
        return *imageResources;
    }

    const vpr::Device* RenderGraph::GetDevice() const noexcept {
        return device;
    }

    static constexpr bool is_buffer_type(const VkDescriptorType& type) {
        if (type == VK_DESCRIPTOR_TYPE_SAMPLER ||
            type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
            type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
            type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
            return false;
        }
        else {
            return true;
        }
    }

    buffer_info_t RenderGraph::createPipelineResourceBufferInfo(const st::ShaderResource * resource) const {
        return buffer_info_t();
    }

    image_info_t RenderGraph::createPipelineResourceImageInfo(const st::ShaderResource * resource) const {
        return image_info_t();
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

}
