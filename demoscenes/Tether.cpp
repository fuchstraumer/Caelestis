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
    void createMeshData();
    void createInstanceData();
    void createIBO();
    void uploadBuffers();
    void createShaders();
    void createPipelineLayout();
    void createPipelineCache();
    void setPipelineState();
    void setVertexPipelineData();
    void createPipeline();

    void createDescriptorPool();

    void updateIBO();
    void updateUBO();

    std::unique_ptr<vpr::Buffer> vbo;
    std::unique_ptr<vpr::Buffer> iboOffsets;
    std::unique_ptr<vpr::Buffer> iboColors;
    std::unique_ptr<vpr::Buffer> ebo;
    std::unique_ptr<vpr::PipelineLayout> pipelineLayout;
    std::unique_ptr<vpr::GraphicsPipeline> pipeline;
    std::unique_ptr<vpr::PipelineCache> cache;
    std::unique_ptr<vpr::ShaderModule> vert, frag;
    VkGraphicsPipelineCreateInfo pipelineCreateInfo;
    vpr::GraphicsPipelineInfo pipelineStateInfo;
    VkViewport viewport;
    VkRect2D scissor;
    std::array<VkVertexInputAttributeDescription, 4> attr;
    std::array<VkVertexInputBindingDescription, 3> bindings;
    size_t currStep = 0;
    bool paused = false;
    void construct();
    void destroy();
    std::vector<uint16_t> indices;
    std::vector<float> meshData;
    std::vector<float> instanceData;
};

constexpr static std::array<const glm::vec3, 6> face_normals{
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 1.0f, 0.0f),
    glm::vec3(-1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f,-1.0f, 0.0f),
    glm::vec3(0.0f, 0.0f,-1.0f)
};

constexpr static std::array<const glm::vec3, 8> vertices{
    glm::vec3(-0.50f,-0.50f, 0.50f),
    glm::vec3(0.50f,-0.50f, 0.50f),
    glm::vec3(0.50f, 0.50f, 0.50f),
    glm::vec3(-0.50f, 0.50f, 0.50f),
    glm::vec3(0.50f,-0.50f,-0.50f),
    glm::vec3(-0.50f,-0.50f,-0.50f),
    glm::vec3(-0.50f, 0.50f,-0.50f),
    glm::vec3(0.50f, 0.50f,-0.50f)
};

constexpr static std::array<glm::vec3, 12> colors{
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

const static std::array<const std::initializer_list<uint16_t>, 6> indices_ilist {
    std::initializer_list<uint16_t>{ 5, 4, 1, 0 },
    std::initializer_list<uint16_t>{ 3, 2, 7, 6 },
    std::initializer_list<uint16_t>{ 4, 5, 6, 7 },
    std::initializer_list<uint16_t>{ 0, 1, 2, 3 },
    std::initializer_list<uint16_t>{ 5, 0, 3, 6 },
    std::initializer_list<uint16_t>{ 1, 4, 7, 2 }
};

TetherScene::TetherScene(const fs::path& path) : BaseScene(2, 1440, 900), offsetData(11) {
    offsetData.SetData(path);
    construct();
}

void TetherScene::construct() {
    createMeshData();
    createVBO();
    createIBO();
    uploadBuffers();
    createShaders();
    createDescriptorPool();
    createPipelineLayout();
    createPipelineCache();
    ubo.projection = GetProjectionMatrix();
    setVertexPipelineData();
    setPipelineState();
    createPipeline();
    setupGUI();
    viewport = VkViewport{ 0.0f,0.0f,1440.0f,900.0f,0.0f,1.0f };
    scissor.offset = VkOffset2D{ 0, 0 };
}

void TetherScene::destroy() {
    vbo.reset();
    ebo.reset();
    iboOffsets.reset();
    iboColors.reset();
    pipelineLayout.reset();
    descriptorPool.reset();
    pipeline.reset();
    cache.reset();
    vert.reset();
    frag.reset();
    gui.reset();
}

TetherScene::~TetherScene() {
    destroy();
}

void TetherScene::WindowResized() {
    destroy();
}

void TetherScene::RecreateObjects() {
    construct();
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

    viewport.width = static_cast<float>(swapchain->Extent.width);
    viewport.height = static_cast<float>(swapchain->Extent.height);

    scissor.extent.width = swapchain->Extent.width;
    scissor.extent.height = swapchain->Extent.height;

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
        const std::vector<VkBuffer> buffers{ vbo->vkHandle(), iboOffsets->vkHandle(), iboColors->vkHandle() };
        static constexpr VkDeviceSize offsets[3]{ 0, 0, 0 };
        vkCmdBindVertexBuffers(scb, 0, static_cast<uint32_t>(buffers.size()), buffers.data(), offsets);
        vkCmdBindIndexBuffer(scb, ebo->vkHandle(), 0, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(scb, indices.size(), offsetData.NumElements, 0, 0, 0);
    vkEndCommandBuffer(scb);
}

void TetherScene::endFrame(const size_t & idx) {
    offsetData.Step();
    updateIBO();
    ++currStep;
}

void TetherScene::timestep_offsets_t::SetData(const fs::path& path) {
    std::string data;

        std::ifstream _file(path, std::ios::in);
        data = std::string{ std::istreambuf_iterator<char>(_file), std::istreambuf_iterator<char>() };
        std::string line;
        _file.seekg(0);
        while (std::getline(_file, line)) {
            ++stepCount;
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

    for (auto& elem : offsets) {
        elem *= 1.0e3f;
    }
}

void TetherScene::createVBO() {
    vbo = std::make_unique<vpr::Buffer>(device.get());
    vbo->CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(float) * meshData.size());
    ebo = std::make_unique<vpr::Buffer>(device.get());
    ebo->CreateBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(uint16_t) * indices.size());
}

void TetherScene::createMeshData() {
    // create mesh face-by-face
    meshData.reserve(36 * 6);
    for (size_t i = 0; i < 6; ++i) {
        for (auto& idx : indices_ilist[i]) {
            meshData.insert(meshData.end(), glm::value_ptr(vertices[idx]), glm::value_ptr(vertices[idx]) + 3);
            meshData.insert(meshData.end(), glm::value_ptr(face_normals[i]), glm::value_ptr(face_normals[i]) + 3);
        }
        indices.insert(indices.end(), indices_ilist[i]);
    }
}

void TetherScene::createInstanceData() {

}

void TetherScene::createIBO() {
    iboOffsets = std::make_unique<vpr::Buffer>(device.get());
    iboOffsets->CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(glm::vec3) * offsetData.NumElements);
    iboColors = std::make_unique<vpr::Buffer>(device.get());
    iboColors->CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(glm::vec3) * offsetData.NumElements);
}

void TetherScene::uploadBuffers() {
    const auto& cmd = transferPool->Begin();
    offsetData.curr = offsetData.offsets.cbegin();
    iboOffsets->CopyTo(offsetData.CurrentAddress(), cmd, sizeof(glm::vec3) * offsetData.NumElements, 0);
    iboColors->CopyTo(colors.data(), cmd, sizeof(glm::vec3) * offsetData.NumElements, 0);
    vbo->CopyTo(meshData.data(), cmd, sizeof(float) * meshData.size(), 0);
    ebo->CopyTo(indices.data(), cmd, sizeof(uint16_t) * indices.size(), 0);
    transferPool->Submit();
}

void TetherScene::createShaders() {
    vert = std::make_unique<vpr::ShaderModule>(device.get(), "../rsrc/shaders/tether/tether.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    frag = std::make_unique<vpr::ShaderModule>(device.get(), "../rsrc/shaders/tether/tether.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
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
    pipelineStateInfo.RasterizationInfo.cullMode = VK_CULL_MODE_NONE;

}

void TetherScene::setVertexPipelineData() {

    bindings[0] = VkVertexInputBindingDescription{
        0, sizeof(glm::vec3) * 2, VK_VERTEX_INPUT_RATE_VERTEX
    };

    bindings[1] = VkVertexInputBindingDescription{
        1, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_INSTANCE
    };

    bindings[2] = VkVertexInputBindingDescription{
        2, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_INSTANCE
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

    attr[3] = VkVertexInputAttributeDescription{
        3, 2, VK_FORMAT_R32G32B32_SFLOAT, 0
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

void TetherScene::createDescriptorPool() {
    descriptorPool = std::make_unique<vpr::DescriptorPool>(device.get(), 1);
    descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);
    descriptorPool->Create();
}

void TetherScene::updateIBO() {
    const auto& cmd = transferPool->Begin();
    iboOffsets->Update(cmd, sizeof(glm::vec3) * offsetData.NumElements, 0, offsetData.CurrentAddress());
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
    scene.RenderLoop();
}