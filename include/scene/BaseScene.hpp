#pragma once
#ifndef VULPES_VK_BASE_SCENE_H
#define VULPES_VK_BASE_SCENE_H
#include "ForwardDecl.hpp"
#include "gui/ImGuiWrapper.hpp"
#include "BaseSceneConfig.hpp"
#include "camera/Camera.hpp"
#include "camera/Arcball.hpp"
#include "glm/mat4x4.hpp"
#include <chrono>
#include <atomic>

namespace vpsk {

    class Window;

    /** Scenes combine the various modules and objects of this library to create a fully renderable scene. Currently only the BaseScene class has been specified,
     *  but this class will simplify much of the setup work required and is extremely easy to extend and use. There is also currently a mostly-untested house demo
     *  scene, showing how this class can be derived from and extended.
     *  \defgroup Scenes
     */

    constexpr static bool WaitForAcquire = false;
    // wait time for 60FPS 
    constexpr static uint64_t AcquireWaitTime = static_cast<uint64_t>((1.0 / 60.0) * 1.0e9);
    // Wait 5 seconds at max
    constexpr static uint64_t AcquireMaxTime = static_cast<uint64_t>(5.0 * 1.0e9);

    /** The BaseScene class takes care of setting up as much as it can, leaving only the calling of the framebuffer and renderpass creation methods to the user. It 
     *  calls suitable virtual methods to signal window resize and swapchain recreation events, and takes care of things like framerate locking (defaults to 60fps) to
     *  avoid overwhelming the CPU. 
     *  An important note is that this uses MSAA by default. Overriding this will require overriding the SetupRenderpass method, to avoid creating the 
     *  various subpasses, attachments, and other Vulkan objects required for this particular type of renderpass setup.
     *  \ingroup Scenes
     *  \todo Find a way to remove the requirement of initializing the renderpass and framebuffers in derived classes, to avoid confusion. 
     */
    class BaseScene {
        BaseScene(const BaseScene&) = delete;
        BaseScene& operator=(const BaseScene&) = delete;
    public:

        /** Creates a new BaseScene, initializing and setting up all but the Framebuffers and the Renderpass. 
         *  \param num_secondary_buffers: the amount of secondary command buffers to allocate. Using one secondary buffer per object is a solid bet.
         *  However, note that internally the actual allocated quantity of command buffers is what is passed in multiplied by the quantity of swapchain images.
         *  \param width: initial width of spawned GLFW window.
         *  \param height: initial height of spawned GLFW window.
        */
        BaseScene(const uint32_t& width, const uint32_t& height);

        ~BaseScene();
         
        /**Shared by all passes.
        */
        virtual void CreateTransferPool();

        virtual void RecreateSwapchain();
        
        /** This method will be called when GLFW receives a window resize event. At this point, all resources created by a derived class should be destroyed.
         *  This includes things like DescriptorPool's, PipelineCache's, and any custom renderable objects that have been used.
         */
        virtual void WindowResized() = 0;
        /** This method is called after WindowResized, and after RecreateSwapchain. At this point, everything is back in order and the Scene is ready to continue rendering.
         *  Now is the time to re-init resources and objects, ensuring to re-upload their data to the device.  
         */
        virtual void RecreateObjects() = 0;

        /** This is called from the RenderLoop() method, and is where commands are recorded into VkCommandBuffer objects that describe the rendering to be done. See the example
         *  in HouseScene for clarification. 
         */
        virtual void RecordCommands() = 0;
        virtual void RenderLoop();

        virtual void UpdateMovement(const float & delta_time);

        void SetCameraTarget(const glm::vec3& target_pos);

        void AddFrameCmdBuffer(const VkCommandBuffer& buffer_to_submit);

        // updates mouse actions via ImGui.
        virtual void UpdateMouseActions();

        static void PipelineCacheCreated(const uint16_t& cache_id);

        glm::mat4 GetViewMatrix() const noexcept;
        glm::mat4 GetProjectionMatrix() const noexcept;
        const glm::vec3& GetCameraPosition() const noexcept;
        
        void SetCameraPosition(const glm::vec3& new_position) noexcept;

        static bool CameraLock;
        static vulpesSceneConfig SceneConfiguration;
        static vpsk_state_t VPSKState;
        static std::atomic<bool> ShouldResize;
    protected:



        virtual void createTransferCmdPool();

        void cleanupShaderCacheFiles();
        virtual void mouseDown(const int& button, const float& x, const float& y);
        virtual void mouseUp(const int& button, const float& x, const float& y);
        virtual void mouseDrag(const int& button, const float& dx, const float& dy);
        virtual void mouseScroll(const int& button, const float& scroll_amount);

        virtual void limitFrame();
        // override in derived classes to perform extra work per frame. Does nothing by default.
        virtual void submitExtra(const uint32_t frame_idx);
        void acquireNextImage(uint32_t* image_idx_ptr);
        virtual void submitFrame(uint32_t* image_idx_ptr);
        void presentFrame(const uint32_t idx);
        // Uses fences to wait for frame completion. Called after submitExtra.
        void waitForFrameComplete(const uint32_t idx);
        // Call ImGui drawing functions (like ImGui::ShowMainMenuBar(), etc) here. Synchronize using contained semaphores.
        virtual void renderGUI(VkCommandBuffer& gui_buffer, const VkCommandBufferBeginInfo& begin_info, const size_t& frame_idx) const;
        
        /** This method is called from the RenderLoop() method as well, before resetting the command pools and continuing to the next frame. Use this
         *  to reset any frame resources, or to wait for fences that are used to guard frame-only commands. Use the frame_idx to ensure that only items
         *  belonging to the presented frame/framebuffer are reset. 
         */
        virtual void endFrame(const size_t& curr_idx) = 0;

        std::unique_ptr<vpsk::ImGuiWrapper> gui;
        uint32_t width, height;
        VkSemaphore semaphores[2];
        std::vector<VkSemaphore> renderCompleteSemaphores;
        std::unique_ptr<vpr::Instance> instance;
        std::unique_ptr<vpr::Device> device;
        std::unique_ptr<vpr::Swapchain> swapchain;
        std::unique_ptr<vpr::TransferPool> transferPool;
        std::unique_ptr<Window> window;
        std::vector<VkCommandBuffer> frameCmdBuffers;

        std::chrono::system_clock::time_point limiter_a, limiter_b;
        double desiredFrameTimeMs = 16.0;
        std::vector<VkFence> presentFences;
        VkFence acquireFence;

        float frameTime;

        static std::vector<uint16_t> pipelineCacheHandles;
        size_t numSecondaryBuffers;
        
        static PerspectiveCamera fpsCamera;
        static ArcballCamera arcballCamera;
    };


}

#endif // !VULPES_VK_BASE_SCENE_H
