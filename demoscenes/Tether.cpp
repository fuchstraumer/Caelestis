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
#include "scene/Window.hpp"
#include "util/easylogging++.h"
INITIALIZE_EASYLOGGINGPP
#include "../tinyobjloader/tiny_obj_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "glm/gtx/hash.hpp"
#include "geometries/Skybox.hpp"
#include "geometries/Icosphere.hpp"
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
    void updatePlots();
    void drawPlots();
    void RecordCommands() final;

    void renderTether(VkCommandBuffer scb, const VkCommandBufferBeginInfo& begin);

private:

    void endFrame(const size_t& idx) final;
    void loadTetherData(const fs::path& tether_data_path = fs::path("tether.dat"));
    void createVBO();
    void createUBO();
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
    void createDescriptorSetLayout();
    void createDescriptorSet();
    void createDescriptorPool();
    void updatePushConstantData();
    void updateUBO();
    uint16_t addVertex(glm::vec3 p, glm::vec3 n);
    void construct();
    void destroy();

    void createSkybox();
    void createMoon();
 

    struct vertex_shader_ubo_t {
        glm::mat4 view, projection;
    } pushConstantData;

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

    struct instance_data_t {
        std::vector<glm::vec4> Offsets;
        instance_data_t(size_t num_elems) {
            Offsets.resize(num_elems);
        }
        void setData(const timestep_offsets_t& vals);
    } uboData;

    struct f_ubo_data_t {
        glm::vec4 viewPos;
        glm::vec4 lightPos;
    } fragmentUboData;

    std::unique_ptr<vpr::Buffer> vbo;
    std::unique_ptr<vpr::Buffer> iboColors;
    std::unique_ptr<vpr::Buffer> ubo, fragmentUBO;
    std::unique_ptr<vpr::Buffer> ebo;
    std::unique_ptr<vpr::PipelineLayout> pipelineLayout;
    std::unique_ptr<vpr::GraphicsPipeline> pipeline;
    std::unique_ptr<vpr::PipelineCache> cache;
    std::unique_ptr<vpr::ShaderModule> vert, frag;
    std::unique_ptr<vpr::DescriptorSet> descriptor;
    std::unique_ptr<vpr::DescriptorSetLayout> setLayout, moonLayout;
    VkGraphicsPipelineCreateInfo pipelineCreateInfo;
    vpr::GraphicsPipelineInfo pipelineStateInfo;
    VkViewport viewport;
    VkRect2D scissor;
    std::array<VkVertexInputAttributeDescription, 3> attr;
    std::array<VkVertexInputBindingDescription, 2> bindings;
    size_t currStep = 0;
    bool paused = false;

    std::vector<uint16_t> indices;
    std::vector<glm::vec3> meshData;
    std::vector<float> instanceData;

    std::unique_ptr<vpsk::Skybox> skybox;
    std::unique_ptr<vpsk::Icosphere> moon;
    
    struct tether_data_t {
        std::vector<float> RadialDistance;
        float MinDist, MaxDist;
        std::vector<float> InPlaneLibration;
        float MinIPL, MaxIPL;
        std::vector<float> OutOfPlaneLibration;
        float MinOOPL, MaxOOPL;
        std::vector<float> Tension;
        float MinTension, MaxTension;
    } tetherData;

    struct tether_data_buffer_t {
        std::array<float*, 2000> Dist;
        std::array<float*, 2000> IPL;
        std::array<float*, 2000> OOPL;
        std::array<float*, 2000> T;
        void rotate() {
            std::rotate(Dist.begin(), Dist.begin() + 1, Dist.end());
            std::rotate(IPL.begin(), IPL.begin() + 1, IPL.end());
            std::rotate(OOPL.begin(), OOPL.begin() + 1, OOPL.end());
            std::rotate(T.begin(), T.begin() + 1, T.end());
        }
    } tetherBuffer;


    int stepsPerFrame = 1;
};

// front 0, right 1, top 2, left 3, bottom 4, back 5

constexpr static std::array<const glm::vec3, 6> face_normals{
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
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
    glm::vec3(0.50f, 0.50f,-0.50f),
};

void getFaceVertices(const size_t i, glm::vec3& v0, glm::vec3& v1, glm::vec3& v2, glm::vec3& v3) {
    switch (i) {
    case 0:
        v0 = vertices[0];
        v1 = vertices[1];
        v2 = vertices[2];
        v3 = vertices[3];
        break;
    case 1:
        v0 = vertices[1];
        v1 = vertices[4];
        v2 = vertices[7];
        v3 = vertices[2];
        break;
    case 2:
        v0 = vertices[3];
        v1 = vertices[2];
        v2 = vertices[7];
        v3 = vertices[6];
        break;
    case 3:
        v0 = vertices[5];
        v1 = vertices[0];
        v2 = vertices[3];
        v3 = vertices[6];
        break;
    case 4:
        v0 = vertices[5];
        v1 = vertices[4];
        v2 = vertices[1];
        v3 = vertices[0];
        break;
    case 5:
        v0 = vertices[4];
        v1 = vertices[5];
        v2 = vertices[6];
        v3 = vertices[7];
        break;
    }
}

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
    std::initializer_list<uint16_t>{ 0, 1, 2, 2, 3, 0 },
    std::initializer_list<uint16_t>{ 3, 2, 6, 6, 7, 3 },
    std::initializer_list<uint16_t>{ 1, 5, 6, 6, 2, 1 },
    std::initializer_list<uint16_t>{ 4, 5, 1, 1, 0, 4 },
    std::initializer_list<uint16_t>{ 4, 0, 3, 3, 7, 4 },
    std::initializer_list<uint16_t>{ 7, 6, 5, 5, 4, 7 },
};

TetherScene::TetherScene(const fs::path& path) : BaseScene(4, 1440, 900), offsetData(11), uboData(11) {
    offsetData.SetData(path);
    loadTetherData();
    construct();
}

void TetherScene::construct() {
    createMeshData();
    createVBO();
    createIBO();
    createUBO();
    uploadBuffers();
    createShaders();
    createDescriptorSetLayout();
    createDescriptorPool();
    createDescriptorSet();
    createPipelineLayout();
    createPipelineCache();
    pushConstantData.projection = GetProjectionMatrix();
    setVertexPipelineData();
    setPipelineState();
    createPipeline();
    setupGUI();
    viewport = VkViewport{ 0.0f,0.0f,1440.0f,900.0f,0.0f,1.0f };
    scissor.offset = VkOffset2D{ 0, 0 };
}

void TetherScene::destroy() {
    skybox.reset();
    gui.reset();
    ubo.reset();
    fragmentUBO.reset();
    descriptor.reset();
    setLayout.reset();
    vbo.reset();
    ebo.reset();
    iboColors.reset();
    pipelineLayout.reset();
    descriptorPool.reset();
    pipeline.reset();
    cache.reset();
    vert.reset();
    frag.reset();
}

void TetherScene::createSkybox() {
    skybox = std::make_unique<vpsk::Skybox>(device.get());
    namespace fs = std::experimental::filesystem;
    fs::path texture("../demoscenes/scene_resources/milkyway_bc3.dds");
    if (!fs::exists(texture)) {
        throw std::runtime_error("Oops.");
    }
    skybox->CreateTexture("../rsrc/milkway_bc3.dds", VK_FORMAT_BC3_UNORM_BLOCK);
    skybox->Create(GetProjectionMatrix(), renderPass->vkHandle(), transferPool.get(), descriptorPool.get());
}

void TetherScene::createMoon() {
    moon = std::make_unique<vpsk::Icosphere>(device.get(), 5, glm::vec3(0.0f), glm::vec3(1.0f));
    moon->CreateShaders(std::string(BaseScene::SceneConfiguration.ResourcePathPrefixStr + "demoscenes/scene_resources/shaders/earthsphere.vert.spv"), std::string(BaseScene::SceneConfiguration.ResourcePathPrefixStr + "demoscenes/scene_resources/shaders/earthsphere.frag.spv"));
    moon->SetTexture("../demoscenes/scene_resources/moon-texture-BC3.dds", VK_FORMAT_BC3_UNORM_BLOCK);
    moon->Init(GetProjectionMatrix(), transferPool.get(), descriptorPool.get(), moonLayout.get());
}

uint16_t TetherScene::addVertex(glm::vec3 p, glm::vec3 n) {
    static uint16_t count = 0;
    meshData.insert(meshData.end(), { std::move(p), std::move(n) });
    return count++;
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

void TetherScene::updatePlots() {
    if (!paused) {
        tetherBuffer.rotate();
        tetherBuffer.Dist.back() = &tetherData.RadialDistance[currStep];
        tetherBuffer.IPL.back() = &tetherData.InPlaneLibration[currStep];
        tetherBuffer.OOPL.back() = &tetherData.OutOfPlaneLibration[currStep];
        tetherBuffer.T.back() = &tetherData.Tension[currStep];
    }
}

void TetherScene::drawPlots() {
    
    if (currStep < 2000) {
        ImGui::ProgressBar(static_cast<float>(currStep) / 2000.0f, ImVec2(ImGui::GetContentRegionAvailWidth(), 30), "Preloading plot data...");
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(450.0f);
            ImGui::TextUnformatted("In order to avoid copies, the plots read from a pointer array of the Tether's data. This array spans from currStep - 2000 to the current step: thus, it's impossible to view until we reach timestep 2000.");
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
    else {
        static float plot_height = 120.0f;
        static bool plotT = true, plotIPL = true;
        static bool plotOOPL = true, plotDist = true;
        ImGui::InputFloat("Plot Window Height", &plot_height, 1.0f, 3.0f);
        if (plot_height > 150.0f) {
            plot_height = 150.0f;
        }
        if (plot_height <= 10.0f) {
            plot_height = 10.0f;
        }
        float avail_width = ImGui::GetContentRegionAvailWidth();
        ImGui::Separator();
        ImGui::Checkbox("Plot Tension", &plotT); ImGui::SameLine();
        ImGui::Checkbox("Plot IPL", &plotIPL); ImGui::SameLine();
        ImGui::Checkbox("Plot OOPL", &plotOOPL); ImGui::SameLine();
        ImGui::Checkbox("Plot R. Dist.", &plotDist);
        if (plotT) {
            ImGui::Separator();
            ImGui::Text("Tension in Tether");
            ImGui::PlotLines("Tension", tetherBuffer.T[0], static_cast<int>(2000), 0, "", tetherData.MinTension, tetherData.MaxTension, ImVec2(ImGui::GetContentRegionAvailWidth(), plot_height));
        }
        if (plotIPL) {
            ImGui::Separator();
            ImGui::Text("In Plane Libration");
            ImGui::PlotLines("In Plane Libration", tetherBuffer.IPL[0], static_cast<int>(2000), 0, "", tetherData.MinIPL, tetherData.MaxIPL, ImVec2(ImGui::GetContentRegionAvailWidth(), plot_height));
        }
        if (plotOOPL) {
            ImGui::Separator();
            ImGui::Text("Out of Plane Libration");
            ImGui::PlotLines("Out of Plane Libration", tetherBuffer.OOPL[0], static_cast<int>(2000), 0, "", tetherData.MinOOPL, tetherData.MaxOOPL, ImVec2(ImGui::GetContentRegionAvailWidth(), plot_height));
        }
        if (plotDist) {
            ImGui::Separator();
            ImGui::Text("Radial Distance");
            ImGui::PlotLines("Radial Distance", tetherBuffer.Dist[0], static_cast<int>(2000), 0, "", tetherData.MinDist, tetherData.MaxDist, ImVec2(ImGui::GetContentRegionAvailWidth(), plot_height));
        }
    }
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

    updatePushConstantData();

    ImGuiIO& io = ImGui::GetIO();

    if ((io.DisplaySize.x != swapchain->Extent.width) || (io.DisplaySize.y != swapchain->Extent.height)) {
        viewport.width = io.DisplaySize.x;
        viewport.height = io.DisplaySize.y;
        scissor.extent.width = static_cast<uint32_t>(io.DisplaySize.x);
        scissor.extent.height = static_cast<uint32_t>(io.DisplaySize.y);
    }
    else {
        viewport.width = static_cast<float>(swapchain->Extent.width);
        viewport.height = static_cast<float>(swapchain->Extent.height);
        scissor.extent.width = swapchain->Extent.width;
        scissor.extent.height = swapchain->Extent.height;
    }

    ImGui::Begin("Static Visualization");
    ImGui::ProgressBar(static_cast<float>(currStep) / static_cast<float>(offsetData.stepCount), ImVec2(ImGui::GetContentRegionAvailWidth(), 30), "Simulation Progess");
    bool pressed = ImGui::Button("Pause");
    if (paused && pressed) {
        paused = false;
    }
    else if (pressed) {
        paused = true;
    }
    if (paused) {
        ImGui::SameLine();
        ImGui::Text("Visualization paused...");
    }
    ImGui::Separator();
    ImGui::Checkbox("Lock mouse to screen", &BaseScene::SceneConfiguration.EnableMouseLocking);
    ImGui::SameLine(); ImGui::InputInt("Data steps per frame", &stepsPerFrame); ImGui::Separator();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(450.0f);
        ImGui::TextUnformatted("This lets you adjust the speed of the simulation, by changing the number of steps (by line) through the input data file per frame rendered. Large values will appear to stutter, potentially.");
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
    if (stepsPerFrame < 1) {
        stepsPerFrame = 1;
    }
    else if (stepsPerFrame > 4) {
        stepsPerFrame = 4;
    }

    drawPlots();
    ImGui::End();

    for (size_t i = 0; i < graphicsPool->size(); ++i) {
        
        secondary_cmd_buffer_inheritance_info.framebuffer = framebuffers[i];
        renderPass->UpdateBeginInfo(framebuffers[i]);

        VkResult err = vkBeginCommandBuffer(graphicsPool->GetCmdBuffer(i), &primary_cmd_buffer_begin_info);
        VkAssert(err);
            vkCmdBeginRenderPass(graphicsPool->GetCmdBuffer(i), &renderPass->BeginInfo(), VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
                auto& scb = secondaryPool->GetCmdBuffer(i * 3);
                renderTether(scb, secondary_cmd_buffer_begin_info);
                renderGUI(secondaryPool->GetCmdBuffer((i * 3) + 1), secondary_cmd_buffer_begin_info, i);
            vkCmdExecuteCommands(graphicsPool->GetCmdBuffer(i), 3, &secondaryPool->GetCmdBuffer(i * 3));
            vkCmdEndRenderPass(graphicsPool->GetCmdBuffer(i));
        err = vkEndCommandBuffer(graphicsPool->GetCmdBuffer(i));
        VkAssert(err);
                
    }
}

void TetherScene::updatePushConstantData() {
    pushConstantData.view = GetViewMatrix();
}

void TetherScene:: updateUBO() {
    ubo->CopyToMapped(uboData.Offsets.data(), sizeof(glm::vec4) * uboData.Offsets.size(), 0);
    fragmentUBO->CopyToMapped(&fragmentUboData, sizeof(glm::vec4) * 2, 0);
}

void TetherScene::renderTether(VkCommandBuffer scb, const VkCommandBufferBeginInfo& begin) {
    vkBeginCommandBuffer(scb, &begin);
        vkCmdBindPipeline(scb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vkHandle());
        vkCmdBindDescriptorSets(scb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout->vkHandle(), 0, 1, &descriptor->vkHandle(), 0, nullptr);
        vkCmdSetViewport(scb, 0, 1, &viewport);
        vkCmdSetScissor(scb, 0, 1, &scissor);
        vkCmdPushConstants(scb, pipelineLayout->vkHandle(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vertex_shader_ubo_t), &pushConstantData);
        const std::vector<VkBuffer> buffers{ vbo->vkHandle(), iboColors->vkHandle() };
        static constexpr VkDeviceSize offsets[2]{ 0, 0 };
        vkCmdBindVertexBuffers(scb, 0, static_cast<uint32_t>(buffers.size()), buffers.data(), offsets);
        vkCmdBindIndexBuffer(scb, ebo->vkHandle(), 0, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(scb, indices.size(), offsetData.NumElements, 0, 0, 0);
    vkEndCommandBuffer(scb);
}

void TetherScene::endFrame(const size_t & idx) {
    if (!paused) {
        for (size_t i = 0; i < static_cast<size_t>(stepsPerFrame); ++i) {
            offsetData.Step();
            updatePlots();
            ++currStep;
        }
        uboData.setData(offsetData);
        fragmentUboData.lightPos = glm::vec4(0.0f, 50.0f, 0.0f, 1.0f);
        const glm::vec3 cpos = GetCameraPosition();
        fragmentUboData.viewPos = glm::vec4(cpos.x, cpos.y, cpos.z, 1.0f);
        updateUBO();
        updatePushConstantData();
    }
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
        if (abs(p.x) <= 1.0e-5f) {
            p.x = 0.0f;
        }
        data_view.remove_prefix(idx+1);
        idx = data_view.find_first_of(' ');
        p.y = strtof(data_view.substr(0,idx).data(),nullptr);
        if (abs(p.y) <= 1.0e-5f) {
            p.y = 0.0f;
        }
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
        if (abs(p.z) <= 1.0e-5f) {
            p.z = 0.0f;
        }
        offsets.push_back(p);   
    }

    offsets.shrink_to_fit();
}

void TetherScene::loadTetherData(const fs::path& tpath) {
    std::ifstream _file(tpath, std::ios::in);
    std::string data{ std::istreambuf_iterator<char>(_file), std::istreambuf_iterator<char>() };
    std::string_view data_view(data);

    while (!data_view.empty()) {
        // Data is: Radial Distance, In Plane Libration, Out of Plane Libration, Tension
        glm::dvec4 p;
        size_t idx = data_view.find_first_of(' ');
        float dist = strtof(data_view.substr(0, idx).data(), nullptr);
        data_view.remove_prefix(idx + 1);
        idx = data_view.find_first_of(' ');
        float ipl = strtof(data_view.substr(0, idx).data(), nullptr);
        data_view.remove_prefix(idx + 1);
        idx = data_view.find_first_of(' ');
        float oopl = strtof(data_view.substr(0, idx).data(), nullptr);
        data_view.remove_prefix(idx + 1);
        idx = data_view.find_first_of(' ');
        size_t n_idx = data_view.find_first_of('\n');
        float tension = 0.0f;
        if (n_idx < idx) {
            tension = strtof(data_view.substr(0, n_idx).data(), nullptr);
            data_view.remove_prefix(n_idx + 1);
        }
        else {
            tension = strtof(data_view.substr(0, idx).data(), nullptr);
            data_view.remove_prefix(idx + 1);
        }

        tetherData.RadialDistance.push_back(dist);
        tetherData.InPlaneLibration.push_back(ipl);
        tetherData.OutOfPlaneLibration.push_back(oopl);
        tetherData.Tension.push_back(tension);
    }

    auto relems = std::minmax_element(tetherData.RadialDistance.cbegin(), tetherData.RadialDistance.cend());
    tetherData.MinDist = *relems.first;
    tetherData.MaxDist = *relems.second;
    auto iip_min_max = std::minmax_element(tetherData.InPlaneLibration.cbegin(), tetherData.InPlaneLibration.cend());
    tetherData.MinIPL = *iip_min_max.first;
    tetherData.MaxIPL = *iip_min_max.second;
    auto oop_min_max = std::minmax_element(tetherData.OutOfPlaneLibration.cbegin(), tetherData.OutOfPlaneLibration.cend());
    tetherData.MinOOPL = *oop_min_max.first;
    tetherData.MaxOOPL = *oop_min_max.second;
    auto t_min_max = std::minmax_element(tetherData.Tension.cbegin(), tetherData.Tension.cend());
    tetherData.MinTension = *t_min_max.first;
    tetherData.MaxTension = *t_min_max.second;

}

void TetherScene::createVBO() {
    vbo = std::make_unique<vpr::Buffer>(device.get());
    vbo->CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(glm::vec3) * meshData.size());
    ebo = std::make_unique<vpr::Buffer>(device.get());
    ebo->CreateBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(uint16_t) * indices.size());
}

void TetherScene::createUBO() {
    ubo = std::make_unique<vpr::Buffer>(device.get());
    ubo->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, offsetData.NumElements * sizeof(glm::vec4));
    fragmentUBO = std::make_unique<vpr::Buffer>(device.get());
    fragmentUBO->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sizeof(glm::vec4) * 2);
}

void TetherScene::createMeshData() {
    for (size_t i = 0; i < 6; ++i) {
        glm::vec3 v0, v1, v2, v3;
        glm::vec3 n = face_normals[i];
        getFaceVertices(i, v0, v1, v2, v3);
        uint16_t i0 = addVertex(v0, n);
        uint16_t i1 = addVertex(v1, n);
        uint16_t i2 = addVertex(v2, n);
        uint16_t i3 = addVertex(v3, n);
        indices.insert(indices.end(), { i0, i1, i2 });
        indices.insert(indices.end(), { i2, i3, i0 });
    }
}

void TetherScene::createIBO() {
    iboColors = std::make_unique<vpr::Buffer>(device.get());
    iboColors->CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(glm::vec3) * offsetData.NumElements);
}

void TetherScene::uploadBuffers() {
    const auto& cmd = transferPool->Begin();
    offsetData.curr = offsetData.offsets.cbegin();
    
    iboColors->CopyTo(colors.data(), cmd, sizeof(glm::vec3) * offsetData.NumElements, 0);
    vbo->CopyTo(meshData.data(), cmd, sizeof(glm::vec3) * meshData.size(), 0);
    ebo->CopyTo(indices.data(), cmd, sizeof(uint16_t) * indices.size(), 0);
    transferPool->Submit();
}

void TetherScene::createShaders() {
    vert = std::make_unique<vpr::ShaderModule>(device.get(), "../rsrc/shaders/tether/tether.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    frag = std::make_unique<vpr::ShaderModule>(device.get(), "../rsrc/shaders/tether/tether.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
}

void TetherScene::createPipelineLayout() {
    pipelineLayout = std::make_unique<vpr::PipelineLayout>(device.get());
    pipelineLayout->Create({ setLayout->vkHandle() }, { VkPushConstantRange{ VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vertex_shader_ubo_t) } });
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

void TetherScene::createDescriptorSetLayout() {
    setLayout = std::make_unique<vpr::DescriptorSetLayout>(device.get());
    setLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);
    setLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
    moonLayout = std::make_unique<vpr::DescriptorSetLayout>(device.get());
    moonLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
}

void TetherScene::createDescriptorSet() {
    descriptor = std::make_unique<vpr::DescriptorSet>(device.get());
    descriptor->AddDescriptorInfo(ubo->GetDescriptor(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);
    descriptor->AddDescriptorInfo(fragmentUBO->GetDescriptor(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    descriptor->Init(descriptorPool.get(), setLayout.get());
}

void TetherScene::createDescriptorPool() {
    descriptorPool = std::make_unique<vpr::DescriptorPool>(device.get(), 5);
    descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3);
    descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
    descriptorPool->Create();
}


void TetherScene::instance_data_t::setData(const timestep_offsets_t & vals) {

    auto vec_iter = Offsets.begin();
    for (auto iter = vals.curr; iter != vals.curr + vals.NumElements; ++iter) {
        vec_iter->x = iter->x;
        vec_iter->y = iter->y;
        vec_iter->z = iter->z;
        vec_iter->w = 0.0f;
        ++vec_iter;
    }

}

int main(int argc, char* argv[]) {
    const std::vector<std::string> args(argv + 1, argv + argc);
    const std::string& input = args.front();
    fs::path in_path(input);
    if (!fs::exists(in_path)) {
        std::cerr << "Invalid path.\n";
        throw std::runtime_error("Invalid path.");
    }
    vpsk::BaseScene::SceneConfiguration.CameraType = vpsk::cameraType::FPS;
    vpsk::BaseScene::SceneConfiguration.ApplicationName = "TetherSim Static Tether Visualization";
    vpsk::BaseScene::SceneConfiguration.EnableMSAA = true;
    vpsk::BaseScene::SceneConfiguration.MSAA_SampleCount = VK_SAMPLE_COUNT_4_BIT;
    vpsk::BaseScene::SceneConfiguration.EnableGUI = true;
    vpsk::BaseScene::SceneConfiguration.EnableMouseLocking = false;
    TetherScene scene(in_path);
    scene.RenderLoop();
    return 0;
}

