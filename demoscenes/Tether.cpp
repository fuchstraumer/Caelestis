#define _HAS_CXX17 1
#include "ForwardDecl.hpp"
#include "scene/BaseScene.hpp"
#include "core/Instance.hpp"
#include "render/GraphicsPipeline.hpp"
#include "render/Renderpass.hpp"
#include "render/Swapchain.hpp"
#include "command/CommandPool.hpp"
#include "command/TransferPool.hpp"
#include "resource/Buffer.hpp"
#include "resource/DescriptorPool.hpp"
#include "resource/DescriptorSet.hpp"
#include "resource/DescriptorSetLayout.hpp"
#include "resource/PipelineLayout.hpp"
#include "resource/ShaderModule.hpp"
#include "resource/Texture.hpp"
#include "resource/Allocator.hpp"
#include "resource/PipelineCache.hpp"
#include "resource/DescriptorPool.hpp"
#include "resource/Semaphore.hpp"
#include "geometries/Skybox.hpp"
#include "util/easylogging++.h"
INITIALIZE_EASYLOGGINGPP
#include "../tinyobjloader/tiny_obj_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "glm/gtx/hash.hpp"
#include <unordered_map>
#include <string>
#include <typeinfo>
#include <experimental/filesystem>
#include <vector>
#include <string_view>


namespace fs = std::experimental::filesystem;

class TetherScene : public vpsk::BaseScene {
public:
    TetherScene(const fs::path& data_file_path);
    ~TetherScene();

    void WindowResized() final;
    void RecreateObjects() final;
    void RecordCommands() final;

    void renderTether(VkCommandBuffer scb, const VkCommandBufferBeginInfo& begin);

private:

    void endFrame(const size_t& idx) final;

    struct instance_data_t {
        glm::vec3 Offset;
    };

    struct vertex_shader_ubo_t {
        glm::mat4 view, projection;
    } ubo;

    struct timestep_offsets_t {
        timestep_offsets_t(const size_t& num_elements) : NumElements(num_elements) {}
        
        void SetData(const fs::path& file);

        void Step() {
            curr += NumElements;
        }

        const void* CurrentAddress() const noexcept {
            return &(*curr);
        }

        
        std::vector<glm::vec3> offsets;
        std::vector<glm::vec3>::const_iterator curr;
        size_t NumElements = 0;
        size_t stepCount = 0;
    } offsetData;

    void createVBO();
    void createIBO();
    void uploadBuffers();
    void createShaders();
    void createPipelineLayout();
    void createPipelineCache();
    void setPipelineState();
    void setVertexData();
    void createPipeline();

    void updateIBO();
    void updateUBO();

    std::unique_ptr<vpr::Buffer> vbo;
    std::unique_ptr<vpr::Buffer> ibo;
    std::unique_ptr<vpr::PipelineLayout> pipelineLayout;
    std::unique_ptr<vpr::GraphicsPipeline> pipeline;
    std::unique_ptr<vpr::PipelineCache> cache;
    std::unique_ptr<vpr::ShaderModule> vert, frag;
    VkGraphicsPipelineCreateInfo pipelineCreateInfo;
    vpr::GraphicsPipelineInfo pipelineStateInfo;
    VkViewport viewport;
    VkRect2D scissor;
    std::array<VkVertexInputAttributeDescription, 2> attr;
    std::array<VkVertexInputBindingDescription, 2> bindings;
    size_t currStep = 0;
    bool paused = false;
};

constexpr static std::array<float, 12> element_vertices {
    -0.5f,-0.5f, 0.0f,
     0.5f,-0.5f, 0.0f,
    -0.5f, 0.5f, 0.0f,
     0.5f, 0.5f, 0.0f
};

TetherScene::TetherScene(const fs::path& path) : BaseScene(1, 1440, 900), offsetData(11) {
    offsetData.SetData(path);
    createVBO();
    createIBO();
    uploadBuffers();
    createShaders();
    createPipelineLayout();
    createPipelineCache();
    ubo.projection = GetProjectionMatrix();
}

TetherScene::~TetherScene()
{
}

void TetherScene::WindowResized()
{
}

void TetherScene::RecreateObjects()
{
}

void TetherScene::RecordCommands() {

    static VkCommandBufferBeginInfo primary_cmd_buffer_begin_info{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        nullptr
    };

    static VkCommandBufferInheritanceInfo secondary_cmd_buffer_inheritance_info = vpr::vk_command_buffer_inheritance_info_base;
    secondary_cmd_buffer_inheritance_info.renderPass = renderPass->vkHandle();
    secondary_cmd_buffer_inheritance_info.subpass = 0;

    static VkCommandBufferBeginInfo secondary_cmd_buffer_begin_info{
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        nullptr,
        VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
        &secondary_cmd_buffer_inheritance_info
    };

    updateUBO();

    ImGui::Begin("Static Visualization");
    ImGui::ProgressBar(static_cast<float>(currStep) / static_cast<float>(offsetData.stepCount));
    if (ImGui::Button("Pause")) {
        paused = true;
        ImGui::SameLine(); 
        ImGui::Text("Visualization paused...");
    }
    ImGui::End();

    for (size_t i = 0; i < graphicsPool->size(); ++i) {
        
        secondary_cmd_buffer_inheritance_info.framebuffer = framebuffers[i];
        renderPass->UpdateBeginInfo(framebuffers[i]);

        VkResult err = vkBeginCommandBuffer(graphicsPool->GetCmdBuffer(i), &primary_cmd_buffer_begin_info);
        VkAssert(err);
            vkCmdBeginRenderPass(graphicsPool->GetCmdBuffer(i), &renderPass->BeginInfo(), VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
                auto& scb = secondaryPool->GetCmdBuffer(i * 2);
                renderTether(scb, secondary_cmd_buffer_begin_info);
                renderGUI(secondaryPool->GetCmdBuffer((i * 2) + 1), secondary_cmd_buffer_begin_info, i);
            vkCmdExecuteCommands(graphicsPool->GetCmdBuffer(i), 2, &secondaryPool->GetCmdBuffer(i * 2));
            vkCmdEndRenderPass(graphicsPool->GetCmdBuffer(i));
        err = vkEndCommandBuffer(graphicsPool->GetCmdBuffer(i));
        VkAssert(err);
                
    }
}

void TetherScene::updateUBO() {
    ubo.view = GetViewMatrix();
}

void TetherScene::renderTether(VkCommandBuffer scb, const VkCommandBufferBeginInfo& begin) {
    vkBeginCommandBuffer(scb, &begin);
        vkCmdBindPipeline(scb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vkHandle());
        vkCmdSetViewport(scb, 0, 1, &viewport);
        vkCmdSetScissor(scb, 0, 1, &scissor);
        vkCmdPushConstants(scb, pipelineLayout->vkHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vertex_shader_ubo_t), &ubo);
        const VkBuffer vbos[2]{ vbo->vkHandle(), ibo->vkHandle() };
        static constexpr VkDeviceSize offsets[2]{ 0, 0 };
        vkCmdBindVertexBuffers(scb, 0, 2, vbos, offsets);
        vkCmdDraw(scb, 4, offsetData.NumElements, 0, 0);
    vkEndCommandBuffer(scb);
}

void TetherScene::endFrame(const size_t & idx) {
    offsetData.Step();
    updateIBO();
    ++currStep;
}

void TetherScene::timestep_offsets_t::SetData(const fs::path& path) {
    std::string data;
    try {
        std::ifstream _file(path, std::ios::in);
        _file.exceptions(std::ios::failbit | std::ios::badbit);
        data = std::string{ std::istreambuf_iterator<char>(_file), std::istreambuf_iterator<char>() };
        std::string line;
        
        while (std::getline(_file, line)) {
            ++stepCount;
        }
    }
    catch (const std::exception& e) {
        throw e;
    }

    std::string_view data_view(data);

    offsets.reserve(stepCount * 12);

    while(!data_view.empty()) {
        glm::vec3 p;
        size_t idx = data_view.find_first_of(' ');
        p.x = strtof(data_view.substr(0,idx).data(),nullptr);
        data_view.remove_prefix(idx+1);
        idx = data_view.find_first_of(' ');
        p.y = strtof(data_view.substr(0,idx).data(),nullptr);
        data_view.remove_prefix(idx+1);
        idx = data_view.find_first_of(' ');
        size_t n_idx = data_view.find_first_of('\n');
        if (n_idx < idx) {
            p.z = strtof(data_view.substr(0, n_idx).data(), nullptr);
            data_view.remove_prefix(n_idx + 1);
        }
        else {
            p.z = strtof(data_view.substr(0, idx).data(), nullptr);
            data_view.remove_prefix(idx + 1);
        }
        offsets.push_back(p);   
    }

    offsets.shrink_to_fit();
}

void TetherScene::createVBO() {
    vbo = std::make_unique<vpr::Buffer>(device.get());
    vbo->CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(float) * 12);   
}

void TetherScene::createIBO() {
    ibo = std::make_unique<vpr::Buffer>(device.get());
    ibo->CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(glm::vec3) * offsetData.NumElements);
}

void TetherScene::uploadBuffers() {
    const auto& cmd = transferPool->Begin();
    ibo->CopyTo(offsetData.CurrentAddress(), cmd, sizeof(glm::vec3) * 12, 0);
    vbo->CopyTo(element_vertices.data(), cmd, sizeof(float) * 12, 0);
    transferPool->Submit();
}

void TetherScene::createShaders() {
    vert = std::make_unique<vpr::ShaderModule>(device.get(), "rsrc/shaders/tether/tether.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    frag = std::make_unique<vpr::ShaderModule>(device.get(), "rsrc/shaders/tether/tether.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
}

void TetherScene::createPipelineLayout() {
    pipelineLayout = std::make_unique<vpr::PipelineLayout>(device.get());
    pipelineLayout->Create({ VkPushConstantRange{ VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vertex_shader_ubo_t) } });
}

void TetherScene::createPipelineCache() {
    cache = std::make_unique<vpr::PipelineCache>(device.get(), static_cast<uint16_t>(typeid(decltype(*this)).hash_code()));
}

void TetherScene::setPipelineState() {

    pipelineStateInfo.VertexInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size());
    pipelineStateInfo.VertexInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attr.size());
    pipelineStateInfo.VertexInfo.pVertexBindingDescriptions = bindings.data();
    pipelineStateInfo.VertexInfo.pVertexAttributeDescriptions = attr.data();
    pipelineStateInfo.DynamicStateInfo.dynamicStateCount = 2;
    constexpr static VkDynamicState states[2]{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    pipelineStateInfo.DynamicStateInfo.pDynamicStates = states;
    if (BaseScene::SceneConfiguration.EnableMSAA) {
        pipelineStateInfo.MultisampleInfo.sampleShadingEnable = VK_TRUE;
        pipelineStateInfo.MultisampleInfo.rasterizationSamples = BaseScene::SceneConfiguration.MSAA_SampleCount;
    }

}

void TetherScene::setVertexData() {

    bindings[0] = VkVertexInputBindingDescription{
        0, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_VERTEX
    };

    bindings[1] = VkVertexInputBindingDescription{
        1, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_INSTANCE
    };

    attr[0] = VkVertexInputAttributeDescription{
        0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0
    };

    attr[1] = VkVertexInputAttributeDescription{
        1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0
    };

}

void TetherScene::createPipeline() {

    pipelineCreateInfo = pipelineStateInfo.GetPipelineCreateInfo();
    pipelineCreateInfo.layout = pipelineLayout->vkHandle();
    pipelineCreateInfo.renderPass = renderPass->vkHandle();

    const VkPipelineShaderStageCreateInfo stages[2]{
        vert->PipelineInfo(),
        frag->PipelineInfo()
    };

    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = stages;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = -1;
    pipelineCreateInfo.subpass = 0;

    pipeline = std::make_unique<vpr::GraphicsPipeline>(device.get());
    pipeline->Init(pipelineCreateInfo, cache->vkHandle());

}

void TetherScene::updateIBO() {
    const auto& cmd = transferPool->Begin();
    ibo->Update(cmd, sizeof(glm::vec3) * offsetData.NumElements, 0, offsetData.CurrentAddress());
    transferPool->Submit();
}

void main(int argc, char* argv[]) {
    const std::vector<std::string> args(argv + 1, argv + argc);
    const std::string& input = args.front();
    fs::path in_path(input);
    if (!fs::exists(in_path)) {
        std::cerr << "Invalid path.\n";
        throw std::runtime_error("Invalid path.");
    }

    TetherScene scene(in_path);
}