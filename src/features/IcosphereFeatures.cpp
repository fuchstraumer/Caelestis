#include "features/IcosphereFeatures.hpp"
#include "resource/DescriptorPool.hpp"
#include "resource/DescriptorSetLayout.hpp"
#include "resource/PipelineLayout.hpp"
#include "resource/ShaderModule.hpp"
#include "scene/BaseScene.hpp"
#include "geometries/vertex_t.hpp"
#include <mutex>
using namespace vpr;

namespace vpsk {

    IcosphereFeatures::IcosphereFeatures(const Device* dvc, const TransferPool* transfer_pool) : device(dvc), transferPool(transfer_pool) {

    }

    void IcosphereFeatures::Init() {
        createSetLayout();
        createPipelineLayout();
        createDescriptorPool();
        createShaders();
        setPipelineStateInfo();
    }

    void IcosphereFeatures::Render(const VkCommandBuffer& cmd) const {
        std::lock_guard<std::mutex> lock(std::mutex());
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vkHandle());
        for(auto obj : objects) {
            obj->Render(cmd, layout->vkHandle());
        }
    }

    void IcosphereFeatures::AddObject(const Icosphere* ico) {
        objects.push_back(ico);
    }

    void IcosphereFeatures::createSetLayout() {
        setLayout = std::make_unique<DescriptorSetLayout>(device);
        setLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void IcosphereFeatures::createDescriptorPool() {
        const size_t num_textures_req = objects.size();
        descriptorPool = std::make_unique<DescriptorPool>(device);
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(num_textures_req));
        descriptorPool->Create();
    }

    void IcosphereFeatures::createPipelineLayout() {
        layout = std::make_unique<PipelineLayout>(device);
        static const std::vector<VkPushConstantRange> push_constants {
            // Multiply matrices on CPU, transfer calculated MVP to shader using push constant.
            VkPushConstantRange{ VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4) }
        };
        layout->Create({ setLayout->vkHandle() }, push_constants);
    }

    void IcosphereFeatures::createShaders() {
        vert = std::make_unique<ShaderModule>(device, BaseScene::SceneConfiguration.ResourcePathPrefixStr + 
            std::string("rsrc/shaders/icosphere.vert.spv"), VK_SHADER_STAGE_VERTEX_BIT);
        frag = std::make_unique<ShaderModule>(device, BaseScene::SceneConfiguration.ResourcePathPrefixStr + 
            std::string("rsrc/shaders/icosphere.frag.spv"), VK_SHADER_STAGE_FRAGMENT_BIT);
    }

    void IcosphereFeatures::setPipelineStateInfo() {

        constexpr static VkDynamicState dynamic_states[2] {
            VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
        };

        pipelineInfo.DynamicStateInfo.dynamicStateCount = 2;
        pipelineInfo.DynamicStateInfo.pDynamicStates = dynamic_states;

        pipelineInfo.MultisampleInfo.sampleShadingEnable = BaseScene::SceneConfiguration.EnableMSAA;
        pipelineInfo.MultisampleInfo.rasterizationSamples = BaseScene::SceneConfiguration.MSAA_SampleCount;

        pipelineInfo.VertexInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_t::bindingDescriptions.size());
        pipelineInfo.VertexInfo.pVertexBindingDescriptions = vertex_t::bindingDescriptions.data();
        pipelineInfo.VertexInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_t::attributeDescriptions.size());
        pipelineInfo.VertexInfo.pVertexAttributeDescriptions = vertex_t::attributeDescriptions.data();

    }

}