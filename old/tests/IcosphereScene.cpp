#include "vpr_stdafx.h"
#include "scene/BaseScene.hpp"
#include "common/CreateInfoBase.hpp"
#include "core/Instance.hpp"
#include "core/LogicalDevice.hpp"
#include "render/Swapchain.hpp"
#include "render/Renderpass.hpp"
#include "command/CommandPool.hpp"
#include "command/TransferPool.hpp"
#include "resource/DescriptorPool.hpp"
#include "resource/Allocator.hpp"
#include "util/UtilitySphere.hpp" 
#include "geometries/Icosphere.hpp"
#include "geometries/Skybox.hpp"
#include "camera/Camera.hpp"
#include "camera/Arcball.hpp"
#include "math/Ray.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "glm/gtx/hash.hpp"
#include "glm/gtc/quaternion.hpp"
#include "util/easylogging++.h"
INITIALIZE_EASYLOGGINGPP



    using namespace vpsk;
    using namespace vpr;

    class IcosphereScene : public BaseScene {
    public:
        
        IcosphereScene();
        ~IcosphereScene();

        virtual void WindowResized() override;
        virtual void RecreateObjects() override;
        virtual void RecordCommands() override;

    private:

        virtual void endFrame(const size_t& idx) override;
        void create();
        void destroy();

        void createSkybox();
        void createEarthSphere();
        void createCloudSphere();
        void createDescriptorPool();
        void updateUBOs();

        std::unique_ptr<Skybox> skybox;
        std::unique_ptr<Icosphere> earthSphere;
        std::unique_ptr<Icosphere> cloudSphere;
        VkViewport viewport = vk_default_viewport;
        VkRect2D scissor = vk_default_viewport_scissor;

    };

    IcosphereScene::IcosphereScene() : BaseScene(4, 1440, 900) {
        create();
        setupGUI();
    }

    IcosphereScene::~IcosphereScene() {
        destroy();
    }

    void IcosphereScene::WindowResized() {
        destroy();
    }

    void IcosphereScene::RecreateObjects() {
        create();
    }

    void IcosphereScene::RecordCommands() {

        static VkCommandBufferBeginInfo primary_cmd_buffer_begin_info {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            nullptr,
            VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            nullptr
        };
    
        static VkCommandBufferInheritanceInfo secondary_cmd_buffer_inheritance_info = vk_command_buffer_inheritance_info_base;
        secondary_cmd_buffer_inheritance_info.renderPass = renderPass->vkHandle();
        secondary_cmd_buffer_inheritance_info.subpass = 0;
    
        static VkCommandBufferBeginInfo secondary_cmd_buffer_begin_info {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            nullptr,
            VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT | VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
            &secondary_cmd_buffer_inheritance_info
        };
    
        updateUBOs();

        ImGui::BeginMainMenuBar();
        ImGui::EndMainMenuBar();
    
        for(size_t i = 0; i < graphicsPool->size(); ++i) {
    
            secondary_cmd_buffer_inheritance_info.framebuffer = framebuffers[i];
            renderPass->UpdateBeginInfo(framebuffers[i]);
    
            viewport.width = static_cast<float>(swapchain->Extent.width);
            viewport.height = static_cast<float>(swapchain->Extent.height);
    
            scissor.extent.width = swapchain->Extent.width;
            scissor.extent.height = swapchain->Extent.height;
    
            VkResult err = vkBeginCommandBuffer(graphicsPool->GetCmdBuffer(i), &primary_cmd_buffer_begin_info);
            VkAssert(err);
    
                vkCmdBeginRenderPass(graphicsPool->GetCmdBuffer(i), &renderPass->BeginInfo(), VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
                    skybox->Render(secondaryPool->GetCmdBuffer(i * 4), secondary_cmd_buffer_begin_info, viewport, scissor);
                    earthSphere->Render(secondaryPool->GetCmdBuffer((i * 4) + 1), secondary_cmd_buffer_begin_info, viewport, scissor);
                    cloudSphere->Render(secondaryPool->GetCmdBuffer((i * 4) + 2), secondary_cmd_buffer_begin_info, viewport, scissor);
                    renderGUI(secondaryPool->GetCmdBuffer((i * 4) + 3), secondary_cmd_buffer_begin_info, i);
                VkCommandBuffer secondary_buffers[4]{ 
                    secondaryPool->GetCmdBuffer(i * 4), secondaryPool->GetCmdBuffer((i * 4) + 1), 
                    secondaryPool->GetCmdBuffer((i * 4) + 2), secondaryPool->GetCmdBuffer((i * 4) + 3) };
                vkCmdExecuteCommands(graphicsPool->GetCmdBuffer(i), 4, secondary_buffers);
                vkCmdEndRenderPass(graphicsPool->GetCmdBuffer(i));
            
            err = vkEndCommandBuffer(graphicsPool->GetCmdBuffer(i));
            VkAssert(err);
            
        }
    }

    void IcosphereScene::endFrame(const size_t & idx) {
        vkResetFences(device->vkHandle(), 1, &presentFences[idx]);
    }

    void IcosphereScene::create() {
        SetCameraPosition(glm::vec3(0.0f, 1.0f, 3.0f));
        SetCameraTarget(glm::vec3(0.0f));
        arcballCamera.SetArcballSphere(UtilitySphere(glm::vec3(0.0f), 1.0f));
        createDescriptorPool();
        createSkybox();
        createEarthSphere();
        createCloudSphere();
    }

    void IcosphereScene::destroy() {
        earthSphere.reset();
        skybox.reset();
        cloudSphere.reset();
        gui.reset();
    }

    void IcosphereScene::createSkybox() {
        LOG(INFO) << "Creating skybox...";
        skybox = std::make_unique<Skybox>(device.get());
        skybox->CreateTexture("../demoscenes/scene_resources/milkyway_bc3.dds", VK_FORMAT_BC3_UNORM_BLOCK);
        skybox->Create(GetProjectionMatrix(), renderPass->vkHandle(), transferPool.get(), descriptorPool.get());
    }

    void IcosphereScene::createEarthSphere() {
        earthSphere = std::make_unique<Icosphere>(4, glm::vec3(0.0f), glm::vec3(1.0f));
        earthSphere->CreateShaders(std::string(BaseScene::SceneConfiguration.ResourcePathPrefixStr + "demoscenes/scene_resources/shaders/earthsphere.vert.spv"), std::string(BaseScene::SceneConfiguration.ResourcePathPrefixStr + "demoscenes/scene_resources/shaders/earthsphere.frag.spv"));
        earthSphere->SetTexture(std::string(BaseScene::SceneConfiguration.ResourcePathPrefixStr + "demoscenes/scene_resources/earth_surface.dds").c_str(), VK_FORMAT_BC3_UNORM_BLOCK);
        earthSphere->Init(GetProjectionMatrix(), renderPass->vkHandle(), transferPool.get(), descriptorPool.get());
    }

    void IcosphereScene::createCloudSphere() {
        cloudSphere = std::make_unique<Icosphere>(4, glm::vec3(0.0f), glm::vec3(1.02f));
        cloudSphere->CreateShaders(BaseScene::SceneConfiguration.ResourcePathPrefixStr + "demoscenes/scene_resources/shaders/earthsphere.vert.spv", BaseScene::SceneConfiguration.ResourcePathPrefixStr + "demoscenes/scene_resources/shaders/earthsphere.frag.spv");
        cloudSphere->SetTexture(std::string(BaseScene::SceneConfiguration.ResourcePathPrefixStr + "demoscenes/scene_resources/earth_clouds.dds").c_str(), VK_FORMAT_BC3_UNORM_BLOCK);
        cloudSphere->Init(GetProjectionMatrix(), renderPass->vkHandle(), transferPool.get(), descriptorPool.get());
    }

    void IcosphereScene::createDescriptorPool() {
        descriptorPool = std::make_unique<DescriptorPool>(device.get(), 4);
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4);
        descriptorPool->Create();
    }

    void IcosphereScene::updateUBOs() {
        skybox->UpdateUBO(GetViewMatrix());
        earthSphere->UpdateUBO(GetViewMatrix(), GetCameraPosition());
        cloudSphere->UpdateUBO(GetViewMatrix(), GetCameraPosition());

        static auto start_time = std::chrono::high_resolution_clock::now();
        auto curr_time = std::chrono::high_resolution_clock::now();
        float diff = std::chrono::duration_cast<std::chrono::milliseconds>(curr_time - start_time).count() / 10000.0f;
        glm::mat4 rot_model = glm::rotate(glm::mat4(1.0f), diff * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // pivot house around center axis based on time.
        earthSphere->SetModelMatrix(rot_model);
        diff *= 0.97f;
        rot_model = glm::scale(glm::mat4(1.0f), glm::vec3(1.04f));
        rot_model = glm::rotate(rot_model, diff * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        cloudSphere->SetModelMatrix(rot_model);

    }



int main() {
    BaseScene::SceneConfiguration.EnableGUI = true;
    BaseScene::SceneConfiguration.EnableMouseLocking = false;
    BaseScene::SceneConfiguration.CameraType = cameraType::ARCBALL;
    BaseScene::SceneConfiguration.ApplicationName = "Arcball DemoScene";
#ifdef _WIN32
    BaseScene::SceneConfiguration.ResourcePathPrefixStr = std::string("../");
#else
    BaseScene::SceneConfiguration.ResourcePathPrefixStr = std::string("../");
#endif
    BaseScene::SceneConfiguration.EnableFullscreen = false;
    IcosphereScene scene;
    scene.RenderLoop();
    return 0;
}