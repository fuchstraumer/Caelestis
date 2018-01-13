#include "features/TetherFeatures.hpp"
#include "scene/BaseScene.hpp"
#include "resource/ShaderModule.hpp"
#include "resource/DescriptorSetLayout.hpp"
#include "glm/vec3.hpp"
#include "imgui/imgui.h"
namespace vpsk {

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

    void TetherFeatures::createShaders() {
        vert = std::make_unique<vpr::ShaderModule>(device, BaseScene::SceneConfiguration.ResourcePathPrefixStr + "rsrc/shaders/tether/tether.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        frag = std::make_unique<vpr::ShaderModule>(device, BaseScene::SceneConfiguration.ResourcePathPrefixStr + "rsrc/shaders/tether/tether.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    }



}