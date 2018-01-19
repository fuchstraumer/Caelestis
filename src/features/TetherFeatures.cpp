#include "features/TetherFeatures.hpp"
#include "scene/BaseScene.hpp"
#include "resource/ShaderModule.hpp"
#include "resource/DescriptorSetLayout.hpp"
#include "resource/DescriptorPool.hpp"
#include "resource/PipelineLayout.hpp"
#include "command/CommandPool.hpp"
#include "glm/vec3.hpp"
#include "imgui/imgui.h"
namespace vpsk {

    const static std::array<const glm::vec3, 12> node_colors {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.9f, 0.0f, 0.1f),
        glm::vec3(0.8f, 0.0f, 0.2f),
        glm::vec3(0.7f, 0.0f, 0.3f),
        glm::vec3(0.6f, 0.0f, 0.4f),
        glm::vec3(0.5f, 0.0f, 0.5f),
        glm::vec3(0.4f, 0.0f, 0.6f),
        glm::vec3(0.3f, 0.0f, 0.7f),
        glm::vec3(0.2f, 0.0f, 0.8f),
        glm::vec3(0.9f, 0.0f, 0.1f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, 0.2f, 0.8f),
    };

    void TetherFeatures::Init() {
        createCommandPools();
        createSetLayout();
        createPipelineLayout();
        createDescriptorPool();
        setVertexPipelineData();
        setPipelineState();
        createShaders();
    }

    void TetherFeatures::AddObject(const Tether* obj) {
        tethers.push_back(obj);
    }

    VkCommandBuffer TetherFeatures::Render(VkRenderPassBeginInfo rp, VkCommandBufferInheritanceInfo cbi) const {
        constexpr static VkCommandBufferBeginInfo begin_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, 
             VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr };
        const VkCommandBufferBeginInfo secondary_begin_info{  VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
             VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
             &cbi };

        VkResult err = vkBeginCommandBuffer(primaryPool->GetCmdBuffer(0), &begin_info); VkAssert(err);
            vkCmdBeginRenderPass(primaryPool->GetCmdBuffer(0), &rp, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
            renderObjects(secondary_begin_info);
            vkCmdExecuteCommands(primaryPool->GetCmdBuffer(0), static_cast<uint32_t>(secondaryPool->size()), secondaryPool->GetCommandBuffers(0));
        err = vkEndCommandBuffer(primaryPool->GetCmdBuffer(0));

        return primaryPool->GetCmdBuffer(0);
    }

    void TetherFeatures::createCommandPools() {
        primaryPool = std::make_unique<vpr::CommandPool>(device);
        primaryPool->AllocateCmdBuffers(1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
        secondaryPool = std::make_unique<vpr::CommandPool>(device);
        secondaryPool->AllocateCmdBuffers(static_cast<uint32_t>(tethers.size()), VK_COMMAND_BUFFER_LEVEL_SECONDARY);
    }

    void TetherFeatures::setVertexPipelineData() {
        bindings[0] = VkVertexInputBindingDescription{
            0, sizeof(glm::vec3) * 2, VK_VERTEX_INPUT_RATE_VERTEX
        };

        bindings[1] = VkVertexInputBindingDescription{
            1, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_INSTANCE
        };

        attr[0] = VkVertexInputAttributeDescription{
            0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0
        };

        attr[1] = VkVertexInputAttributeDescription{
            1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3
        };

        attr[2] = VkVertexInputAttributeDescription{
            2, 1, VK_FORMAT_R32G32B32_SFLOAT, 0
        };
    }

    void TetherFeatures::setPipelineState() {

        pipelineStateInfo.VertexInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size());
        pipelineStateInfo.VertexInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attr.size());
        pipelineStateInfo.VertexInfo.pVertexBindingDescriptions = bindings.data();
        pipelineStateInfo.VertexInfo.pVertexAttributeDescriptions = attr.data();

        if (BaseScene::SceneConfiguration.EnableMSAA) {
            pipelineStateInfo.MultisampleInfo.sampleShadingEnable = VK_TRUE;
            pipelineStateInfo.MultisampleInfo.rasterizationSamples = BaseScene::SceneConfiguration.MSAA_SampleCount;
        }

        pipelineStateInfo.RasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        pipelineStateInfo.ViewportInfo.scissorCount = 1;
        pipelineStateInfo.ViewportInfo.viewportCount = 1;

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

        pipelineStateInfo.ViewportInfo.pViewports = &viewport;
        pipelineStateInfo.ViewportInfo.pScissors = &scissor;
    }

    void TetherFeatures::createSetLayout() {
        setLayout = std::make_unique<vpr::DescriptorSetLayout>(device);
        setLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
        setLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    }

    void TetherFeatures::createDescriptorPool() {
        descriptorPool = std::make_unique<vpr::DescriptorPool>(device, static_cast<uint32_t>(tethers.size()));
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2 * static_cast<uint32_t>(tethers.size()));
        descriptorPool->Create();
    }

    void TetherFeatures::createPipelineLayout() {
        pipelineLayout = std::make_unique<vpr::PipelineLayout>(device);
        pipelineLayout->Create({ setLayout->vkHandle() }, { VkPushConstantRange{ VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vertex_shader_ubo_t)} });
    }
    
    void TetherFeatures::createShaders() {
        vert = std::make_unique<vpr::ShaderModule>(device, BaseScene::SceneConfiguration.ResourcePathPrefixStr + "rsrc/shaders/tether/tether.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        frag = std::make_unique<vpr::ShaderModule>(device, BaseScene::SceneConfiguration.ResourcePathPrefixStr + "rsrc/shaders/tether/tether.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    }



}