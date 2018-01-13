#include "features/IcosphereFeatures.hpp"
#include "resource/DescriptorPool.hpp"
#include "resource/DescriptorSetLayout.hpp"
#include "resource/PipelineLayout.hpp"
#include "resource/ShaderModule.hpp"
#include "scene/BaseScene.hpp"
#include "command/CommandPool.hpp"
#include "geometries/vertex_t.hpp"
#include "imgui/imgui.h"
#include <future>
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

    VkCommandBuffer IcosphereFeatures::Render(VkRenderPassBeginInfo rp, VkCommandBufferInheritanceInfo cbi) const {
        constexpr static VkCommandBufferBeginInfo begin_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, 
             VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr };
        const VkCommandBufferBeginInfo secondary_begin_info{  VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
             VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
             &cbi };
        
        VkResult err = vkBeginCommandBuffer(primaryPool->GetCmdBuffer(0), &begin_info); VkAssert(err);
            vkCmdBeginRenderPass(primaryPool->GetCmdBuffer(0), &rp, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
            renderObjects(secondary_begin_info);
            vkCmdExecuteCommands(primaryPool->GetCmdBuffer(0), static_cast<uint32_t>(secondaryPool->size()), secondaryPool->GetCommandBuffers(0));
            vkCmdEndRenderPass(primaryPool->GetCmdBuffer(0));
        err = vkEndCommandBuffer(primaryPool->GetCmdBuffer(0)); VkAssert(err);

        return primaryPool->GetCmdBuffer(0);
    }

    void IcosphereFeatures::renderObjects(const VkCommandBufferBeginInfo& info) const {

        const VkPipelineLayout& layout_handle = layout->vkHandle();
        const auto render_object = [info,layout_handle](const Icosphere* ico, VkCommandBuffer cmd) {
            vkBeginCommandBuffer(cmd, &info);
            ico->Render(cmd, layout_handle);
            vkEndCommandBuffer(cmd);
        };

        std::vector<std::future<void>> futures;
        size_t i = 0;
        for (auto& obj : objects) {
            futures.push_back(std::async(std::launch::async, render_object, obj, secondaryPool->GetCmdBuffer(i)));
            ++i;
        }

        for (auto&& fut : futures) {
            fut.get();
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

        pipelineInfo.DynamicStateInfo.dynamicStateCount = 0;
        pipelineInfo.DynamicStateInfo.pDynamicStates = nullptr;

        pipelineInfo.MultisampleInfo.sampleShadingEnable = BaseScene::SceneConfiguration.EnableMSAA;
        pipelineInfo.MultisampleInfo.rasterizationSamples = BaseScene::SceneConfiguration.MSAA_SampleCount;

        pipelineInfo.VertexInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_t::bindingDescriptions.size());
        pipelineInfo.VertexInfo.pVertexBindingDescriptions = vertex_t::bindingDescriptions.data();
        pipelineInfo.VertexInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_t::attributeDescriptions.size());
        pipelineInfo.VertexInfo.pVertexAttributeDescriptions = vertex_t::attributeDescriptions.data();

        pipelineInfo.ViewportInfo.scissorCount = 1;
        pipelineInfo.ViewportInfo.viewportCount = 1;

        const ImGuiIO& io = ImGui::GetIO();

        viewport = VkViewport{ 
            0.0f, 0.0f,
            io.DisplaySize.x, io.DisplaySize.y,
            0.0f, 1.0f
        };

        scissor.extent.width = static_cast<uint32_t>(io.DisplaySize.x);
        scissor.extent.height = static_cast<uint32_t>(io.DisplaySize.y);
        scissor.offset.x = 0;
        scissor.offset.y = 0;

        pipelineInfo.ViewportInfo.pViewports = &viewport;
        pipelineInfo.ViewportInfo.pScissors = &scissor;

    }

}