#include "geometries/Billboard.hpp"
#include "resource/Texture.hpp"
#include "resource/Buffer.hpp"
#include "resource/ShaderModule.hpp"
#include "resource/DescriptorSet.hpp"
#include "resource/DescriptorSetLayout.hpp"
#include "resource/PipelineLayout.hpp"
#include "resource/PipelineCache.hpp"
#include "render/GraphicsPipeline.hpp"
#include "core/Instance.hpp"
#include "core/LogicalDevice.hpp"
#include "command/TransferPool.hpp"
#include "scene/BaseScene.hpp"

using namespace vpr;

namespace vpsk {

    constexpr static std::array<float, 20> billboard_vertices {
        -0.5f,-0.5f, 0.0f, 0.0f, 1.0f,
         0.5f,-0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 0.0f,
         0.5f, 0.5f, 0.0f, 1.0f, 0.0f
    };

    Billboard::Billboard(const glm::vec3& position_, const glm::vec3& scale_, const glm::mat4& projection_) : position(position_), scale(scale_) {
        uboData.projection = projection_;
    }

    Billboard::~Billboard() {
        vbo.reset();
        frag.reset();
        vert.reset();
        pipelineLayout.reset();
        pipelineCache.reset();
        pipeline.reset();
        texture.reset();
    }

    void Billboard::SetSurfaceTexture(const char* texture_filename) {

        texture = std::make_unique<Texture<texture_2d_t>>(device);
        texture->CreateFromFile(texture_filename);

    }

    void Billboard::Init(const Device* dvc, const VkRenderPass& renderpass, TransferPool* transfer_pool, DescriptorPool* descriptor_pool) {
        device = dvc;
        createBuffers();
        transferResources(transfer_pool);
        createShaders();
        createDescriptorSet(descriptor_pool);
        createPipelineLayout();
        createPipelineCache();
        setPipelineStateInfo();
        createGraphicsPipeline(renderpass);
    }

    void Billboard::Render(const VkCommandBuffer& cmd, const VkCommandBufferBeginInfo& begin_info, const VkViewport& viewport, const VkRect2D& scissor) {
    
        vkBeginCommandBuffer(cmd, &begin_info);
        {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vkHandle());
            vkCmdSetViewport(cmd, 0, 1, &viewport);
            vkCmdSetScissor(cmd, 0, 1, &scissor);
            vkCmdPushConstants(cmd, pipelineLayout->vkHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uboData), &uboData);
            static const VkDeviceSize offsets[1]{ 0 };
            vkCmdBindVertexBuffers(cmd, 0, 1, &vbo->vkHandle(), offsets);
            vkCmdDraw(cmd, 4, 1, 0, 1);
        }
        vkEndCommandBuffer(cmd);
    }

    void Billboard::UpdateUBO(const glm::mat4& view) {
        uboData.view = view;
        uboData.cameraUp = glm::vec4(view[0][1], view[1][1], view[2][1], 0.0f);
        uboData.cameraRight = glm::vec4(view[0][0], view[1][0], view[2][0], 1.0f);
    }

    void Billboard::createBuffers() {

        vbo = std::make_unique<Buffer>(device);
        vbo->CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(float) * billboard_vertices.size());
        
    }

    void Billboard::transferResources(TransferPool* transfer_pool_) {

        auto& cmd = transfer_pool_->Begin();
            vbo->CopyTo(const_cast<float*>(billboard_vertices.data()), cmd, sizeof(float) * 15, 0);
            texture->TransferToDevice(cmd);
        transfer_pool_->Submit();

    }

    void Billboard::createShaders() {

        vert = std::make_unique<ShaderModule>(device, "rsrc/shaders/billboard.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        frag = std::make_unique<ShaderModule>(device, "rsrc/shaders/billboard.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    }

    void Billboard::createDescriptorSet(DescriptorPool* descriptor_pool) {

        setLayout = std::make_unique<DescriptorSetLayout>(device);
        setLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_VERTEX_BIT, 0);

        descriptorSet = std::make_unique<DescriptorSet>(device);

        descriptorSet->AddDescriptorInfo(texture->GetDescriptor(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0);
        descriptorSet->Init(descriptor_pool, setLayout.get());

    }

    void Billboard::createPipelineLayout() {

        pipelineLayout = std::make_unique<PipelineLayout>(device);
        pipelineLayout->Create( { setLayout->vkHandle() }, { VkPushConstantRange{ VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(billboard_ubo_data_t) } });
    
    }

    void Billboard::createPipelineCache() {

        pipelineCache = std::make_unique<PipelineCache>(device, static_cast<uint16_t>(typeid(Billboard).hash_code()));

    }

    void Billboard::setPipelineStateInfo() {

        constexpr static VkDynamicState dynamic_states[2]{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        pipelineStateInfo.DynamicStateInfo.dynamicStateCount = 2;
        pipelineStateInfo.DynamicStateInfo.pDynamicStates = dynamic_states;

        pipelineStateInfo.MultisampleInfo.sampleShadingEnable = BaseScene::SceneConfiguration.EnableMSAA;
        if (pipelineStateInfo.MultisampleInfo.sampleShadingEnable) {
            pipelineStateInfo.MultisampleInfo.rasterizationSamples = BaseScene::SceneConfiguration.MSAA_SampleCount;
        }

        
        pipelineStateInfo.DepthStencilInfo.depthTestEnable = VK_TRUE;
        pipelineStateInfo.DepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        pipelineStateInfo.AssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

        pipelineStateInfo.VertexInfo.vertexBindingDescriptionCount = 1;
        pipelineStateInfo.VertexInfo.pVertexBindingDescriptions = &bindingDescription;
        pipelineStateInfo.VertexInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        pipelineStateInfo.VertexInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    }

    void Billboard::createGraphicsPipeline(const VkRenderPass& renderpass) {

        const VkPipelineShaderStageCreateInfo shader_stages[2] { vert->PipelineInfo(), frag->PipelineInfo() };
        pipelineCreateInfo = pipelineStateInfo.GetPipelineCreateInfo();
        pipelineCreateInfo.stageCount = 2;
        pipelineCreateInfo.pStages = shader_stages;
        pipelineCreateInfo.subpass = 0;
        pipelineCreateInfo.layout = pipelineLayout->vkHandle();


    }

}