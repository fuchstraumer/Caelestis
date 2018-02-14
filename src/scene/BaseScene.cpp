#include "scene/BaseScene.hpp"
#include "scene/Window.hpp"
#include "scene/InputHandler.hpp"
#include "core/Instance.hpp"
#include "core/LogicalDevice.hpp"
#include "core/PhysicalDevice.hpp"
#include "command/CommandPool.hpp"
#include "command/TransferPool.hpp"
#include "render/Framebuffer.hpp"
#include "render/Renderpass.hpp"
#include "render/Swapchain.hpp"
#include "render/DepthStencil.hpp"
#include "render/Multisampling.hpp"
#include "render/GraphicsPipeline.hpp"
#include "resource/Buffer.hpp"
#include "resource/DescriptorPool.hpp"
#include "resource/DescriptorSet.hpp"
#include "resource/DescriptorSetLayout.hpp"
#include "resource/PipelineLayout.hpp"
#include "resource/ShaderModule.hpp"
#include "alloc/Allocator.hpp"
#include "util/easylogging++.h"
#ifdef USE_EXPERIMENTAL_FILESYSTEM
#include <experimental/filesystem>
#endif
#include <thread>

using namespace vpr;

namespace vpsk {

    vulpesSceneConfig BaseScene::SceneConfiguration = vulpesSceneConfig();
    bool BaseScene::CameraLock = false;
    PerspectiveCamera BaseScene::fpsCamera = PerspectiveCamera(1440, 900, 70.0f);
    ArcballCamera BaseScene::arcballCamera = ArcballCamera(1440, 900, 70.9f, UtilitySphere(glm::vec3(0.0f), 7.0f));
    vpsk_state_t BaseScene::VPSKState = vpsk_state_t();
    std::atomic<bool> BaseScene::ShouldResize(false);

    std::vector<uint16_t> BaseScene::pipelineCacheHandles = std::vector<uint16_t>();

    BaseScene::BaseScene(const uint32_t& _width, const uint32_t& _height) : width(_width), height(_height) {

        const bool verbose_logging = BaseScene::SceneConfiguration.VerboseLogging;
        ImGui::CreateContext();

        window = std::make_unique<Window>(_width, _height, BaseScene::SceneConfiguration.ApplicationName);
        const VkApplicationInfo app_info{
            VK_STRUCTURE_TYPE_APPLICATION_INFO,
            nullptr,
            BaseScene::SceneConfiguration.ApplicationName.c_str(),
            VK_MAKE_VERSION(0,1,0),
            BaseScene::SceneConfiguration.EngineName.c_str(),
            VK_MAKE_VERSION(0,1,0),
            VK_API_VERSION_1_0
        };
        instance = std::make_unique<Instance>(false, &app_info, window->glfwWindow());
        window->SetWindowUserPointer(this);

        LOG_IF(verbose_logging, INFO) << "VkInstance created.";

        Multisampling::SampleCount = BaseScene::SceneConfiguration.MSAA_SampleCount;
        LOG_IF(verbose_logging, INFO) << "MSAA level set to " << std::to_string(Multisampling::SampleCount);

        device = std::make_unique<Device>(instance.get(), instance->GetPhysicalDevice(), true);

        swapchain = std::make_unique<Swapchain>(instance.get(), device.get());


        LOG_IF(verbose_logging, INFO) << "Swapchain created.";

        VkSemaphoreCreateInfo semaphore_info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0 };
        VkResult result = vkCreateSemaphore(device->vkHandle(), &semaphore_info, nullptr, &semaphores[0]);
        VkAssert(result);
        result = vkCreateSemaphore(device->vkHandle(), &semaphore_info, nullptr, &semaphores[1]);
        VkAssert(result);

        renderCompleteSemaphores.resize(1);
        result = vkCreateSemaphore(device->vkHandle(), &semaphore_info, nullptr, &renderCompleteSemaphores[0]);
        VkAssert(result);

        presentFences.resize(swapchain->ImageCount());
    
        VkFenceCreateInfo fence_info = vk_fence_create_info_base;
        for (auto& fence : presentFences) {
            result = vkCreateFence(device->vkHandle(), &fence_info, nullptr, &fence);
            VkAssert(result);
        }

        result = vkCreateFence(device->vkHandle(), &fence_info, nullptr, &acquireFence);
        VkAssert(result);

        // init frame limiters.
        limiter_a = std::chrono::system_clock::now();
        limiter_b = std::chrono::system_clock::now();

        input_handler::LastX = swapchain->Extent().width / 2.0f;
        input_handler::LastY = swapchain->Extent().height / 2.0f;

        if (SceneConfiguration.CameraType == cameraType::FPS) {
            fpsCamera.SetNearClipPlaneDistance(0.1f);
            fpsCamera.SetFarClipPlaneDistance(2000.0f);
        }
        else if (SceneConfiguration.CameraType == cameraType::ARCBALL) {
            arcballCamera.SetNearClipPlaneDistance(0.1f);
            arcballCamera.SetFarClipPlaneDistance(2000.0f);
        }

        LOG_IF(verbose_logging, INFO) << "BaseScene setup complete.";

        transferPool = std::make_unique<TransferPool>(device.get());
    }

    BaseScene::~BaseScene() {
        gui.reset();
        transferPool.reset();
        swapchain.reset();

        for (const auto& fence : presentFences) {
            vkDestroyFence(device->vkHandle(), fence, nullptr);
        }

        vkDestroyFence(device->vkHandle(), acquireFence, nullptr);

        vkDestroySemaphore(device->vkHandle(), semaphores[1], nullptr);
        vkDestroySemaphore(device->vkHandle(), semaphores[0], nullptr);

#ifdef USE_EXPERIMENTAL_FILESYSTEM
        cleanupShaderCacheFiles();
#endif
        window.reset();
    }
    
    

    void BaseScene::UpdateMouseActions() {

        ImGuiIO& io = ImGui::GetIO();

        if (!io.WantCaptureMouse) {

            if (ImGui::IsMouseDragging(0)) {
                mouseDrag(0, io.MousePos.x, io.MousePos.y);
            }
            else if (ImGui::IsMouseDragging(1)) {
                mouseDrag(1, io.MousePos.x, io.MousePos.y);
            }

            if (ImGui::IsMouseDown(0) && !ImGui::IsMouseDragging(0)) {
                mouseDown(0, io.MouseClickedPos[0].x, io.MouseClickedPos[0].y);
            }

            if (ImGui::IsMouseDown(1) && !ImGui::IsMouseDragging(1)) {
                mouseDown(1, io.MouseClickedPos[1].x, io.MouseClickedPos[1].y);
            }

            if (ImGui::IsMouseReleased(0)) {
                mouseUp(0, io.MousePos.x, io.MousePos.y);
            }

            mouseScroll(0, io.MouseWheel);
        }

        input_handler::MouseDx = 0.0f;
        input_handler::MouseDy = 0.0f;
    }

    void BaseScene::mouseDrag(const int& button, const float & rot_x, const float & rot_y) {
        if (SceneConfiguration.CameraType == cameraType::FPS) {
            fpsCamera.MouseDrag(button, rot_x, rot_y);
        }
        else if (SceneConfiguration.CameraType == cameraType::ARCBALL) {
            arcballCamera.MouseDrag(button, rot_x, rot_y);
        }

        ImGui::ResetMouseDragDelta();
    }

    void BaseScene::mouseScroll(const int& button, const float & zoom_delta) {
        if (SceneConfiguration.CameraType == cameraType::FPS) {
            fpsCamera.MouseScroll(button, zoom_delta);
        }
        else if (SceneConfiguration.CameraType == cameraType::ARCBALL) {
            arcballCamera.MouseScroll(button, zoom_delta);
        }
    }

    void BaseScene::mouseDown(const int& button, const float& x, const float& y) {
        if (SceneConfiguration.CameraType == cameraType::FPS) {
            fpsCamera.MouseDown(button, x, y);
        }
        else if (SceneConfiguration.CameraType == cameraType::ARCBALL) {
            arcballCamera.MouseDown(button, x, y);
        }
    }

    void BaseScene::mouseUp(const int& button, const float & x, const float & y) {
        if (SceneConfiguration.CameraType == cameraType::FPS) {
            fpsCamera.MouseUp(button, x, y);
        }
        else if (SceneConfiguration.CameraType == cameraType::ARCBALL) {
            arcballCamera.MouseUp(button, x, y);
        }
    }

    void BaseScene::PipelineCacheCreated(const uint16_t & cache_id) {
        pipelineCacheHandles.push_back(cache_id);
    }

    void BaseScene::createTransferCmdPool() {
        VkCommandPoolCreateInfo pool_info = vk_command_pool_info_base;
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pool_info.flags = device->QueueFamilyIndices.Transfer;
        transferPool = std::make_unique<TransferPool>(device.get());
    }

    void BaseScene::CreateTransferPool() {

        LOG(INFO) << "Creating primary graphics and transfer command pools...";
        createTransferCmdPool();

    }

    void BaseScene::RecreateSwapchain() {

        LOG_IF(BaseScene::SceneConfiguration.VerboseLogging, INFO) << "Began to recreate swapchain...";

        // First wait to make sure nothing is in use.
        vkDeviceWaitIdle(device->vkHandle());
        transferPool.reset();
        WindowResized();


        device->vkAllocator->Recreate();

        vpr::RecreateSwapchainAndSurface(instance.get(), swapchain.get());
        device->VerifyPresentationSupport();
        /*
            Done destroying resources, recreate resources and objects now
        */

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize.x = static_cast<float>(swapchain->Extent().width);
        io.DisplaySize.y = static_cast<float>(swapchain->Extent().height);
        BaseScene::fpsCamera = PerspectiveCamera(swapchain->Extent().width, swapchain->Extent().height, 70.0f);
        BaseScene::arcballCamera = ArcballCamera(swapchain->Extent().width, swapchain->Extent().height, 70.0f, UtilitySphere(glm::vec3(0.0f), 7.0f));
        input_handler::LastX = swapchain->Extent().width / 2.0f;
        input_handler::LastY = swapchain->Extent().height / 2.0f;
        if (SceneConfiguration.CameraType == cameraType::FPS) {
            fpsCamera.SetNearClipPlaneDistance(0.1f);
            fpsCamera.SetFarClipPlaneDistance(2000.0f);
        }
        else if (SceneConfiguration.CameraType == cameraType::ARCBALL) {
            arcballCamera.SetNearClipPlaneDistance(0.1f);
            arcballCamera.SetFarClipPlaneDistance(2000.0f);
        }

        RecreateObjects();
        vkDeviceWaitIdle(device->vkHandle());

        LOG_IF(BaseScene::SceneConfiguration.VerboseLogging, INFO) << "Swapchain recreation successful.";

    }

    void BaseScene::RenderLoop() {

        frameTime = static_cast<float>(BaseScene::SceneConfiguration.FrameTimeMs / 1000.0);
        LOG(INFO) << "Entering rendering loop.";

        while(!glfwWindowShouldClose(window->glfwWindow())) {

            glfwPollEvents();

            if (ShouldResize.exchange(false)) {
                RecreateSwapchain();
            }

            limitFrame();

            UpdateMouseActions();

            if(SceneConfiguration.EnableGUI) {
                gui->NewFrame(window->glfwWindow());
            }

            UpdateMovement(static_cast<float>(BaseScene::SceneConfiguration.FrameTimeMs));

            // Acquire image, which will run async while we record commands
            uint32_t idx = 0;
            acquireNextImage(&idx);
            if (idx == std::numeric_limits<uint32_t>::max()) {
                continue;
            }
            RecordCommands();
            // Submit recorded commands. Waits for semaphores specified when acquiring images.
            submitFrame(&idx);

            submitExtra(idx);
            waitForFrameComplete(idx);
            endFrame(idx);

            Buffer::DestroyStagingResources(device.get());

        }
    }

    void BaseScene::UpdateMovement(const float& delta_time) {

        frameTime = delta_time;
        ImGuiIO& io = ImGui::GetIO();
        io.DeltaTime = delta_time;

        if (io.WantCaptureKeyboard) {
            return;
        }

        if (input_handler::Keys[GLFW_KEY_W]) {
            if (SceneConfiguration.CameraType == cameraType::FPS) {
                fpsCamera.SetEyeLocation(fpsCamera.GetEyeLocation() + SceneConfiguration.MovementSpeed * fpsCamera.GetViewDirection() * 0.1f);
            }
        }

        if (input_handler::Keys[GLFW_KEY_A]) {
            if (SceneConfiguration.CameraType == cameraType::FPS) {
                fpsCamera.SetEyeLocation(fpsCamera.GetEyeLocation() - SceneConfiguration.MovementSpeed * fpsCamera.GetRightDirection() * 0.1f);
            }
        }

        if (input_handler::Keys[GLFW_KEY_D]) {
            if (SceneConfiguration.CameraType == cameraType::FPS) {
                fpsCamera.SetEyeLocation(fpsCamera.GetEyeLocation() + SceneConfiguration.MovementSpeed * fpsCamera.GetRightDirection() * 0.1f);
            }
        }

        if (input_handler::Keys[GLFW_KEY_S]) {
            if (SceneConfiguration.CameraType == cameraType::FPS) {
                fpsCamera.SetEyeLocation(fpsCamera.GetEyeLocation() - SceneConfiguration.MovementSpeed * fpsCamera.GetViewDirection() * 0.1f);
            }
        }

    }

    void BaseScene::limitFrame() {

        if(!BaseScene::SceneConfiguration.LimitFramerate) {
            return;
        }

        limiter_a = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> work_time = limiter_a - limiter_b;
        
        if(work_time.count() < BaseScene::SceneConfiguration.FrameTimeMs) {
            std::chrono::duration<double, std::milli> delta_ms(BaseScene::SceneConfiguration.FrameTimeMs - work_time.count());
            auto delta_ms_dur = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
            std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_dur.count()));
        }

        limiter_b = std::chrono::system_clock::now();

    }

    void BaseScene::submitExtra(const uint32_t frame_idx) {

        return;
    }


    void BaseScene::acquireNextImage(uint32_t* image_idx_ptr) {
        VkResult result = VK_SUCCESS;
        result = vkAcquireNextImageKHR(device->vkHandle(), swapchain->vkHandle(), AcquireMaxTime, semaphores[0], acquireFence, image_idx_ptr);
        switch (result) {
        case VK_ERROR_OUT_OF_DATE_KHR:
            LOG(WARNING) << "Received VK_ERROR_OUT_OF_DATE_KHR on trying to acquire an image, recreating swapchain...";
            RecreateSwapchain();
            *image_idx_ptr = std::numeric_limits<uint32_t>::max();
        case VK_SUBOPTIMAL_KHR:
            LOG(WARNING) << "Received VK_SUBOPTIMAL_KHR on trying to acquire an image, recreating swapchain...";
            RecreateSwapchain();
            *image_idx_ptr = std::numeric_limits<uint32_t>::max();
        default:
            VkAssert(result);
            break;
        }
        
        if (WaitForAcquire) {
            result = vkWaitForFences(device->vkHandle(), 1, &acquireFence, VK_TRUE, AcquireWaitTime);
            VkAssert(result);
        }
    }

    void BaseScene::submitFrame(uint32_t* image_idx_ptr) {

        VkSubmitInfo submit_info = vk_submit_info_base;
        VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT };
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &semaphores[0];
        submit_info.pWaitDstStageMask = wait_stages;
        submit_info.commandBufferCount = static_cast<uint32_t>(frameCmdBuffers.size());
        submit_info.pCommandBuffers = frameCmdBuffers.data();
        submit_info.signalSemaphoreCount = static_cast<uint32_t>(renderCompleteSemaphores.size());
        submit_info.pSignalSemaphores = renderCompleteSemaphores.data();
        VkResult result = vkQueueSubmit(device->GraphicsQueue(), 1, &submit_info, presentFences[*image_idx_ptr]);
        switch(result) {
            case VK_ERROR_DEVICE_LOST:
                LOG(WARNING) << "vkQueueSubmit returned VK_ERROR_DEVICE_LOST";
                break;
            default:
                VkAssert(result);
                break;
        }

    }

    void BaseScene::presentFrame(const uint32_t idx) {
        VkPresentInfoKHR present_info{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &renderCompleteSemaphores[0];
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain->vkHandle();
        present_info.pImageIndices = &idx;
        present_info.pResults = nullptr;
        VkResult result = vkQueuePresentKHR(device->GraphicsQueue(), &present_info);
        VkAssert(result);
    }

    void BaseScene::waitForFrameComplete(const uint32_t idx) {
        VkResult result = vkWaitForFences(device->vkHandle(), 1, &presentFences[idx], VK_TRUE, 2);
        VkAssert(result);
        result = vkResetFences(device->vkHandle(), 1, &acquireFence);
        VkAssert(result);
        result = vkResetFences(device->vkHandle(), 1, &presentFences[idx]);
        VkAssert(result);
    }

    void BaseScene::renderGUI(VkCommandBuffer& gui_buffer, const VkCommandBufferBeginInfo& begin_info, const size_t& frame_idx) const {

        if(!SceneConfiguration.EnableGUI) {
            LOG(ERROR) << "Tried to render the GUI, when the GUI is disabled in the SceneConfiguration!";
            throw std::runtime_error("Tried to render the GUI, when the GUI is disabled in the SceneConfiguration!");
        }

        ImGui::Render();

        gui->UpdateBuffers();

        vkBeginCommandBuffer(gui_buffer, &begin_info);
        gui->DrawFrame(gui_buffer);
        vkEndCommandBuffer(gui_buffer);
    }

    glm::mat4 BaseScene::GetViewMatrix() const noexcept {
        if (SceneConfiguration.CameraType == cameraType::FPS) {
            return fpsCamera.GetViewMatrix();
        }
        else if (SceneConfiguration.CameraType == cameraType::ARCBALL) {
            return arcballCamera.GetViewMatrix();
        }
        else {
            LOG(ERROR) << "Invalid camera type detected: defaulting to FPS camera.";
            return fpsCamera.GetViewMatrix();
        }
    }

    glm::mat4 BaseScene::GetProjectionMatrix() const noexcept {
        if (SceneConfiguration.CameraType == cameraType::FPS) {
            return fpsCamera.GetProjectionMatrix();
        }
        else if (SceneConfiguration.CameraType == cameraType::ARCBALL) {
            return arcballCamera.GetProjectionMatrix();
        }
        else {
            LOG(ERROR) << "Invalid camera type detected: defaulting to FPS camera.";
            return fpsCamera.GetProjectionMatrix();
        }
    }

    const glm::vec3 & BaseScene::GetCameraPosition() const noexcept {
        if (SceneConfiguration.CameraType == cameraType::FPS) {
            return fpsCamera.GetEyeLocation();
        }
        else if (SceneConfiguration.CameraType == cameraType::ARCBALL) {
            return arcballCamera.GetEyeLocation();
        }
        else {
            LOG(ERROR) << "Invalid camera type detected: defaulting to FPS camera.";
            return fpsCamera.GetEyeLocation();
        }
    }

    void BaseScene::SetCameraPosition(const glm::vec3& new_camera_pos) noexcept {
        if (SceneConfiguration.CameraType == cameraType::FPS) {
            fpsCamera.SetEyeLocation(new_camera_pos);
        }
        else if (SceneConfiguration.CameraType == cameraType::ARCBALL) {
            arcballCamera.SetEyeLocation(new_camera_pos);
        }
        else {
            LOG(ERROR) << "Invalid camera type detected: defaulting to FPS camera.";
            fpsCamera.SetEyeLocation(new_camera_pos);
        }
    }

    void BaseScene::SetCameraTarget(const glm::vec3& target_pos) {
        if (SceneConfiguration.CameraType == cameraType::FPS) {
            fpsCamera.LookAtTarget(target_pos);
        }
        else if (SceneConfiguration.CameraType == cameraType::ARCBALL) {
            arcballCamera.LookAtTarget(target_pos);
        }
        else {
            LOG(ERROR) << "Invalid camera type detected: defaulting to FPS camera.";
            fpsCamera.LookAtTarget(target_pos);
        }
    }

    void BaseScene::AddFrameCmdBuffer(const VkCommandBuffer & buffer_to_submit) {
        frameCmdBuffers.push_back(buffer_to_submit);
    }
}
