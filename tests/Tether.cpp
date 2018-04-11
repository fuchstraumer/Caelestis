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
#include "pass/Phong.hpp"
#include "geometries/Skybox.hpp"
#include "geometries/Icosphere.hpp"
#include "features/IcosphereFeatures.hpp"
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
    void updatePlots();
    void drawPlots();
    void RecordCommands() final;

    void renderTether(VkCommandBuffer scb, const VkCommandBufferBeginInfo& begin);

private:

    void endFrame(const size_t& idx) final;
    void loadTetherData(const fs::path& tether_data_path = fs::path("tether.dat"));

    void construct();
    void destroy();

    void createSkybox();
    void createMoon();
 
    
    std::unique_ptr<vpr::DescriptorSet> descriptor;

    std::unique_ptr<vpsk::PhongPass> phongPass;
    std::unique_ptr<vpsk::IcosphereFeatures> icospheres;
    
    bool paused = false;

    std::unique_ptr<vpsk::Skybox> skybox;
    std::unique_ptr<vpsk::Icosphere> moon;
    
    


    int stepsPerFrame = 1;
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


TetherScene::TetherScene(const fs::path& path) : BaseScene(1440, 900) {

    construct();
}

void TetherScene::construct() {


}

void TetherScene::destroy() {
    skybox.reset();
    gui.reset();
    descriptor.reset();
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


void TetherScene::RecordCommands() {

    updatePushConstantData();

    ImGuiIO& io = ImGui::GetIO();

    

    drawPlots();
    ImGui::End();


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

void TetherScene::createShaders() {
    vert = std::make_unique<vpr::ShaderModule>(device.get(), "../rsrc/shaders/tether/tether.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    frag = std::make_unique<vpr::ShaderModule>(device.get(), "../rsrc/shaders/tether/tether.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
}

void TetherScene::createPipelineLayout() {
    pipelineLayout = std::make_unique<vpr::PipelineLayout>(device.get());
    pipelineLayout->Create({ setLayout->vkHandle() }, { VkPushConstantRange{ VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vertex_shader_ubo_t) } });
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

