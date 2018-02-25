#include "scene/BaseScene.hpp"
#include "common/CreateInfoBase.hpp"
#include "core/Instance.hpp"
#include "core/LogicalDevice.hpp"
#include "resource/DescriptorPool.hpp"
#include "resource/Buffer.hpp"
#include "util/UtilitySphere.hpp" 
#include "math/Ray.hpp"
#include "util/AABB.hpp"
#include "lights/Light.hpp"
#include "render/Swapchain.hpp"
#include "command/CommandPool.hpp"
#include "command/TransferPool.hpp"
#include "geometries/vertex_t.hpp"
#include "render/GraphicsPipeline.hpp"
#include "render/Renderpass.hpp"
#include "render/Framebuffer.hpp"
#include "resource/ShaderModule.hpp"
#include "resource/PipelineCache.hpp"
#include "resource/PipelineLayout.hpp"
#include "resource/DescriptorSetLayout.hpp"
#include "resource/DescriptorSet.hpp"
#include "resources/TexturePool.hpp"
#include "geometries/ObjModel.hpp"
#include "scene/Window.hpp"
#include "resources/Multisampling.hpp"
#include "render/DepthStencil.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "resources/LightBuffers.hpp"
#include "render/FrameData.hpp"
#include "render/BackBuffer.hpp"
#include <memory>
#include <map>
#include <random>
#include <deque>

#include "util/easylogging++.h"
INITIALIZE_EASYLOGGINGPP

namespace vpsk {

    // https://mynameismjp.wordpress.com/2016/03/25/bindless-texturing-for-deferred-rendering-and-decals/

    using namespace vpr;

    const glm::mat4 clip{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f };
    const glm::mat4 proj = glm::perspective(glm::radians(60.0f), 1.333f, 0.1f, 3000.0f);

    struct program_state_t {
        uint32_t ResolutionX = 1920;
        uint32_t ResolutionY = 1080;
        uint32_t MinLights = 1024;
        uint32_t MaxLights = 2048;
        uint32_t NumLights = 2048;
        bool GenerateLights = false;
        uint32_t TileWidth = 64;
        uint32_t TileHeight = 64;
        uint32_t TileCountX = (1920 - 1) / 64 + 1;
        uint32_t TileCountY = (1080 - 1) / 64 + 1;
        uint32_t TileCountZ = 256;
    } ProgramState;

    struct lights_soa_t {
        void update(const AABB& bounds) {

            const auto set_from_vec = [](const glm::vec4& v) {
                glm::vec3 el(0.0f);
                el.x = glm::length(v);
                if (v.x != 0.0f) {
                    el.y = acosf(glm::clamp(-v.y / el.x, -1.0f, 1.0f));
                    el.z = atan2f(-v.z, v.x);
                }
                return el;
            };

            const auto get_vec = [](const glm::vec3& v) {
                return glm::vec3{
                    v.x * sinf(v.y) * cos(v.z),
                   -v.x * cosf(v.y),
                   -v.x * sinf(v.y) * sinf(v.z)
                };
            };

            const auto restrict_phi = [](const float& y) {
                return std::fmaxf(0.0001f, std::fminf(3.14159f - 0.0001f, y));
            };

            for (size_t i = 0; i < Positions.size(); ++i) {
                auto el = set_from_vec(Positions[i]);
                el.z += 0.001f;
                el.y = restrict_phi(el.y);
                auto v = get_vec(el);
                
                Positions[i].x = v.x;
                Positions[i].y = v.y;
                Positions[i].z = v.z;
            }

        }
        std::vector<glm::vec4> Positions;
        std::vector<glm::u8vec4> Colors;
    } Lights;

    std::uniform_real_distribution<float> rand_distr(0.0f, 1.0f);
    std::random_device rd;
    std::mt19937 rng(rd());

    float random_unit_float() {
        return rand_distr(rng);
    }

    float random_range(const float l, const float h) {
        return l + (h - l) * random_unit_float();
    }

    glm::vec3 hue_to_rgb(const float hue) {
        const float s = hue * 6.0f;
        const float r0 = glm::clamp(s - 4.0f, 0.0f, 1.0f);
        const float g0 = glm::clamp(s - 0.0f, 0.0f, 1.0f);
        const float b0 = glm::clamp(s - 2.0f, 0.0f, 1.0f);
        const float r1 = glm::clamp(2.0f - s, 0.0f, 1.0f);
        const float g1 = glm::clamp(4.0f - s, 0.0f, 1.0f);
        const float b1 = glm::clamp(6.0f - s, 0.0f, 1.0f);

        return glm::vec3{ r0 + r1, g0 * g1, b0 * b1 };
    }

    glm::u8vec4 float_to_uchar(const glm::vec3& color) {
        return glm::u8vec4{ 
            static_cast<uint8_t>(std::round(color.x * 255.0f)),
            static_cast<uint8_t>(std::round(color.y * 255.0f)),
            static_cast<uint8_t>(std::round(color.z * 255.0f)),
            0
        };
    }

    void GenerateLights(const AABB& model_bounds) {
        const glm::vec3 extents = model_bounds.Extents();
        const float volume = extents.x * extents.y * extents.z;
        const float light_vol = volume / static_cast<float>(ProgramState.MaxLights);
        const float base_range = powf(light_vol, 1.0f / 3.0f);
        const float max_range = base_range * 3.0f;
        const float min_range = base_range / 1.5f;
        const glm::vec3 half_size = (model_bounds.Max() - model_bounds.Min()) * 0.50f;
        const float pos_radius = std::max(half_size.x, std::max(half_size.y, half_size.z));
        Lights.Positions.reserve(ProgramState.MaxLights);
        Lights.Colors.reserve(ProgramState.MaxLights);
        for (uint32_t i = 0; i < ProgramState.MaxLights; ++i) {
            glm::vec3 fcol = hue_to_rgb(random_range(0.0f,1.0f));
            fcol *= 1.30f;
            fcol -= 0.15f;
            const glm::u8vec4 color = float_to_uchar(fcol);
            const glm::vec4 position{ 
                random_range(-pos_radius, pos_radius), random_range(-pos_radius, pos_radius), random_range(-pos_radius, pos_radius), random_range(min_range, max_range)
            };
            Lights.Positions.push_back(position);
            Lights.Colors.push_back(color);
        }
    }

    struct light_particles_t {
        light_particles_t(const light_particles_t&) = delete;
        light_particles_t& operator=(const light_particles_t&) = delete;
        light_particles_t(const Device* dvc, const lights_soa_t* lights);
        void update();
        std::unique_ptr<Buffer> Positions;
        std::unique_ptr<Buffer> Colors;
        const Device* Device;
        const lights_soa_t* lightsData;
    };

    constexpr static std::array<const VkClearValue, 3> OnscreenClearValues {
        VkClearValue{ 0.1f, 0.1f, 0.15f, 1.0f },
        VkClearValue{ 0.1f, 0.1f, 0.15f, 1.0f },
        VkClearValue{ 1.0f, 0 }
    };

    constexpr static std::array<const VkClearValue, 1> OffscreenClearValues{
        VkClearValue{ 1.0f, 0 }
    };

    struct clustered_forward_global_ubo_t {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
        glm::mat4 normal;
        glm::vec4 viewPosition;
        glm::vec2 depth{ 0.1f, 3000.0f };
        uint32_t NumLights;
    } GlobalUBO;

    struct specialization_constants_t {
        uint32_t ResolutionX = 1920;
        uint32_t ResolutionY = 1080;
        uint32_t TileCountX = 0;
        uint32_t TileCountY = 0;
        uint32_t TileWidth = 64;
        uint32_t TileHeight = 64;
        uint32_t TileCountZ = 256;
    } SpecializationConstants;

    constexpr static std::array<VkSpecializationMapEntry, 11> SpecializationConstantMap {
        VkSpecializationMapEntry{ 0, 0, sizeof(uint32_t) },
        VkSpecializationMapEntry{ 1, sizeof(uint32_t), sizeof(uint32_t) },
        VkSpecializationMapEntry{ 2, sizeof(uint32_t) * 2, sizeof(uint32_t) },
        VkSpecializationMapEntry{ 3, sizeof(uint32_t) * 3, sizeof(uint32_t) },
        VkSpecializationMapEntry{ 4, sizeof(uint32_t) * 4, sizeof(uint32_t) },
        VkSpecializationMapEntry{ 5, sizeof(uint32_t) * 5, sizeof(uint32_t) },
        VkSpecializationMapEntry{ 6, sizeof(uint32_t) * 6, sizeof(uint32_t) },
    };

    struct g_pipeline_items_t {
        void destroy() {
            Cache.reset();
            Layout.reset();
            Pipeline.reset();
        }
        std::unique_ptr<GraphicsPipeline> Pipeline;
        std::unique_ptr<PipelineLayout> Layout;
        std::unique_ptr<PipelineCache> Cache;
    };

    static const std::map<const std::string, const size_t> NameIdxMap { 
            { "LightGrids", 0 }, { "GridOffsets", 1 }, { "LightList", 2 }
    };

    struct compute_pipelines_t {
        void destroy(const Device* dvc) {
            Cache.reset();
            for (auto& handle : Handles) {
                vkDestroyPipeline(dvc->vkHandle(), handle, nullptr);
            }
        }
        std::array<VkPipeline, 3> Handles;
        std::array<VkComputePipelineCreateInfo, 3> Infos;
        std::unique_ptr<PipelineCache> Cache;
    };

    struct clustered_forward_pipelines_t {
        g_pipeline_items_t Depth;
        g_pipeline_items_t LightingOpaque;
        g_pipeline_items_t LightingTransparent;
        g_pipeline_items_t Opaque;
        g_pipeline_items_t Transparent;
        g_pipeline_items_t Particles;
        compute_pipelines_t ComputePipelines;
        void destroy(const Device* dvc) {
            Depth.destroy();
            LightingOpaque.destroy();
            LightingTransparent.destroy();
            Opaque.destroy();
            Transparent.destroy();
            Particles.destroy();
            ComputePipelines.destroy(dvc);
            ComputeLayout.reset();
        }
        std::unique_ptr<PipelineLayout> ComputeLayout;
    };

    class ClusteredForward : public BaseScene {
    public:

        void UpdateUBO();

        ClusteredForward(const std::string& obj_file);
        void create();
        void destroy();
        void RecordCommands() final;
        void RenderLoop() final;

    private:

        void WindowResized() final {}
        void RecreateObjects() final {}
        void endFrame(const size_t& curr_idx) final {}

        void updateUniforms();

        void acquireBackBuffer();
        void recordPrecomputePass(FrameData& frame);
        void submitPrecomputePass(FrameData & frame);
        void recordComputePass(FrameData & frame);
        void submitComputePass(FrameData & frame);
        void recordOnscreenPass(FrameData & frame);
        void submitOnscreenPass(FrameData & frame);
        void resetTexelBuffers(const VkCommandBuffer cmd);
        void renderParticles(const VkCommandBuffer cmd);
        void recordCommands();
        void presentBackBuffer();

        void createShaders();
        void createDepthPipeline();
        void createLightingPipelines();
        void createMainPipelines();
        void createParticlePipeline();
        void createComputePipelines();

        void createComputeCmdPool();
        void createPrimaryCmdPool();

        void createDescriptorPool();
        void createFrameDataSetLayout();
        void createTexelBufferSetLayout();
        void createTexelBufferDescriptorSet();

        void createFrameData();
        void createBackbuffers();
        void createUBO();
        void createLightBuffers();

        void createOnscreenAttachmentDescriptions();
        void createOnscreenAttachmentReferences();
        void createOnscreenSubpassDescription();
        void createOnscreenSubpassDependencies();
        void createOnscreenPass();

        void createComputeAttachmentDescriptions();
        void createComputeSubpassDescriptions();
        void createComputeSubpassDependencies();
        void createComputePass();

        void createOffscreenRenderTarget();
        void createOffscreenFramebuffer();

        void createRenderTargets();

        std::unique_ptr<Multisampling> msaa;
        std::unique_ptr<DepthStencil> depthStencil;
        
        std::unique_ptr<LightBuffers> lightData;
        clustered_forward_pipelines_t Pipelines;
        std::unique_ptr<DescriptorPool> descriptorPool;
        std::unique_ptr<DescriptorSetLayout> frameDataLayout;
        std::unique_ptr<DescriptorSetLayout> texelBuffersLayout;
        std::unique_ptr<CommandPool> primaryPool;
        std::unique_ptr<CommandPool> computePool;
        std::unique_ptr<Image> offscreenRenderTarget;
        std::unique_ptr<Framebuffer> offscreenFramebuffer;

        std::unique_ptr<DescriptorSet> texelBufferSet;

        std::unique_ptr<Renderpass> onscreenPass;
        VkSubpassDescription onscreenSbDescr;
        std::array<VkAttachmentDescription, 3> onscreenDescr;
        std::array<VkAttachmentReference, 3> onscreenReferences;
        std::array<VkSubpassDependency, 2> onscreenDependencies;

        std::unique_ptr<Renderpass> computePass;
        std::array<VkAttachmentDescription, 1> computeAttachmentDescr;
        std::array<VkSubpassDescription, 2> computeSbDescr;
        VkAttachmentReference computeRef;
        std::array<VkSubpassDependency, 3> computeDependencies;
        uint32_t CurrFrameDataIdx = 0;
        std::vector<FrameData> frames;
        std::deque<std::unique_ptr<BackBuffer>> Backbuffers;
        std::unique_ptr<BackBuffer> acquiredBackBuffer;
        VkSubmitInfo OffscreenSubmit, ComputeSubmit, RenderSubmit;
        std::map<std::string, std::unique_ptr<ShaderModule>> shaders;
        VkBufferMemoryBarrier Barriers[2]{ vk_buffer_memory_barrier_info_base, vk_buffer_memory_barrier_info_base };
        std::unique_ptr<Buffer> ubo;
        std::unique_ptr<ObjModel> sponza;
        std::unique_ptr<TexturePool> texturePool;
        std::string objFilename;
        VkViewport offscreenViewport, onscreenViewport;
        VkRect2D offscreenScissor, onscreenScissor;
        std::unique_ptr<light_particles_t> Particles;

        void createVulkanLightData();

        std::unique_ptr<Buffer> LightPositions;
        std::unique_ptr<Buffer> LightColors;
        std::unique_ptr<DescriptorSet> frameDescriptor;

        constexpr static VkPipelineStageFlags OffscreenFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        constexpr static VkPipelineStageFlags ComputeFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        constexpr static VkPipelineStageFlags RenderFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        std::vector<VkFramebuffer> framebuffers;
        
    };

    void ClusteredForward::UpdateUBO() {
        ubo->CopyToMapped(&GlobalUBO, sizeof(GlobalUBO), 0);
    }

    ClusteredForward::ClusteredForward(const std::string & obj_file) : BaseScene(1920, 1080), objFilename(obj_file) {
        create();
    }

    void ClusteredForward::create() {
        ProgramState.TileCountX = (ProgramState.ResolutionX - 1) / ProgramState.TileWidth + 1;
        ProgramState.TileCountY = (ProgramState.ResolutionY - 1) / ProgramState.TileHeight + 1;
        SpecializationConstants.TileCountX = ProgramState.TileCountX;
        SpecializationConstants.TileCountY = ProgramState.TileCountY;

        texturePool = std::make_unique<TexturePool>(device.get(), transferPool.get());
        sponza = std::make_unique<ObjModel>(device.get(), texturePool.get());
        sponza->LoadModelFromFile(objFilename, transferPool.get());
        GenerateLights(sponza->GetAABB());
        createBackbuffers();
        createShaders();
        createUBO();
        createLightBuffers();
        createVulkanLightData();
        createDescriptorPool();
        createComputeCmdPool();
        createPrimaryCmdPool();
        createFrameData();
        createFrameDataSetLayout();
        createTexelBufferSetLayout();
        createTexelBufferDescriptorSet();
        createComputePass();
        createOnscreenPass();
        createDepthPipeline();
        createLightingPipelines();
        createMainPipelines();
        createParticlePipeline();
        createOffscreenRenderTarget();
        createOffscreenFramebuffer();
        createComputePipelines();
        createRenderTargets();
        Particles = std::make_unique<light_particles_t>(device.get(), &Lights);
        ImGuiWrapper wrapper(device.get(), onscreenPass->vkHandle(), descriptorPool.get());
        gui = std::make_unique<ImGuiWrapper>(device.get(), onscreenPass->vkHandle(), descriptorPool.get());
        gui->UploadTextureData(transferPool.get());
    }

    void ClusteredForward::destroy() {
        gui.reset();
        offscreenFramebuffer.reset();
        offscreenRenderTarget.reset();
        Pipelines.destroy(device.get());
        texelBufferSet.reset();
        texelBuffersLayout.reset();
        frameDataLayout.reset();

        primaryPool.reset();
        computePool.reset();
        descriptorPool.reset();
        ubo.reset();
        shaders.clear();
        sponza.reset();
        Backbuffers.clear();
        msaa.reset();
        depthStencil.reset();
        computePass.reset();
        onscreenPass.reset();
        for (auto& fbuff : framebuffers) {
            vkDestroyFramebuffer(device->vkHandle(), fbuff, nullptr);
        }
        lightData->Destroy();
    }

    void ClusteredForward::RecordCommands() {

    }

    void ClusteredForward::RenderLoop() {

        offscreenViewport.width = static_cast<float>(swapchain->Extent().width);
        offscreenViewport.height = static_cast<float>(swapchain->Extent().height);
        offscreenViewport.minDepth = 0.0f;
        offscreenViewport.maxDepth = 1.0f;
        offscreenViewport.x = 0;
        offscreenViewport.y = 0;
        offscreenScissor.offset.x = 0;
        offscreenScissor.offset.y = 0;
        offscreenScissor.extent.width = swapchain->Extent().width;
        offscreenScissor.extent.height = swapchain->Extent().height;
        const glm::vec3 eye = sponza->GetAABB().Center() + glm::vec3(0.0f, 50.0f, 50.0f);
        SetCameraPosition(eye);
        camera.SetFOV(75.0f);
        camera.SetDepthRange(0.1f, 3000.0f);
        GlobalUBO.view = GetViewMatrix();
        GlobalUBO.model = sponza->GetModelMatrix();
        GlobalUBO.normal = glm::transpose(glm::inverse(GlobalUBO.model));
        
        

        Buffer::DestroyStagingResources(device.get());
        static bool first_frame = true;

        while (!glfwWindowShouldClose(window->glfwWindow())) {
            glfwPollEvents();
            camera.MouseMoveUpdate();
            if (ShouldResize.exchange(false) && !first_frame) {
                RecreateSwapchain();
            }

            if (SceneConfiguration.EnableGUI) {
                gui->NewFrame(window->glfwWindow());
            }

            limitFrame();
            UpdateMouseActions();
            UpdateMovement(BaseScene::SceneConfiguration.FrameTimeMs / 1000.0f);
            
            acquireBackBuffer();
            recordCommands();
            presentBackBuffer();
            first_frame = false;
        }
    }

    void ClusteredForward::updateUniforms() {
        GlobalUBO.view = GetViewMatrix();
        GlobalUBO.projection = GetProjectionMatrix();
        GlobalUBO.viewPosition = glm::vec4(GetCameraPosition(), 1.0f);
        GlobalUBO.NumLights = ProgramState.NumLights;
        GlobalUBO.depth.x = camera.GetNearPlane();
        GlobalUBO.depth.y = camera.GetFarPlane();
        ubo->CopyToMapped(&GlobalUBO, sizeof(GlobalUBO), 0);
    }

    void ClusteredForward::acquireBackBuffer() {
        static bool first_frame = true;
        auto& curr = Backbuffers.front();

        vkAcquireNextImageKHR(device->vkHandle(), swapchain->vkHandle(), static_cast<uint64_t>(1e9), curr->GetSemaphore("ImageAcquire"), VK_NULL_HANDLE, curr->GetImageIdxPtr());
        if (!first_frame) {
            Backbuffers.push_back(std::move(acquiredBackBuffer));
        }
        acquiredBackBuffer = std::move(curr);
        Backbuffers.pop_front();
        first_frame = false;
    }

    void ClusteredForward::recordPrecomputePass(FrameData& frame) {
        auto& block = frame.GetCommandBlock("Offscreen");
        VkResult result = vkWaitForFences(device->vkHandle(), 1, &block.Fence, VK_TRUE, static_cast<uint64_t>(3.0e9)); VkAssert(result);
        vkResetFences(device->vkHandle(), 1, &block.Fence);
        if (!block.firstFrame) {
            vkResetCommandBuffer(block.Cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        }

        updateUniforms();

        Barriers[0].buffer = ubo->vkHandle();
        Barriers[0].size = ubo->Size();
        Barriers[0].srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        Barriers[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        offscreenViewport.maxDepth = 1.0f;
        computePass->UpdateBeginInfo(offscreenFramebuffer->vkHandle());
        auto& cmd = block.Cmd;
        constexpr VkCommandBufferBeginInfo begin_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr };
        vkBeginCommandBuffer(cmd, &begin_info);
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, Barriers, 0, nullptr);
        vkCmdBeginRenderPass(cmd, &computePass->BeginInfo(), VK_SUBPASS_CONTENTS_INLINE);
            vkCmdSetViewport(cmd, 0, 1, &offscreenViewport);
            vkCmdSetScissor(cmd, 0, 1, &offscreenScissor);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines.Depth.Layout->vkHandle(), 0, 1, &frameDescriptor->vkHandle(), 0, nullptr);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines.Depth.Pipeline->vkHandle());
            sponza->Render(DrawInfo{ cmd, Pipelines.Depth.Layout->vkHandle(), true, false, 0 });
        vkCmdNextSubpass(cmd, VK_SUBPASS_CONTENTS_INLINE);
            const VkDescriptorSet descriptors[2]{ frameDescriptor->vkHandle(), texelBufferSet->vkHandle() };
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines.LightingOpaque.Layout->vkHandle(), 0, 2, descriptors, 0, nullptr);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines.LightingOpaque.Pipeline->vkHandle());
            sponza->Render(DrawInfo{ cmd, Pipelines.LightingOpaque.Layout->vkHandle(), true, false, 2 });
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines.LightingTransparent.Pipeline->vkHandle());
            sponza->Render(DrawInfo{ cmd, Pipelines.LightingOpaque.Layout->vkHandle(), false, false, 2 });
        vkCmdEndRenderPass(cmd);
        vkEndCommandBuffer(cmd);

        block.firstFrame = false;
    }

    void ClusteredForward::submitPrecomputePass(FrameData& frame) {
        auto& block = frame.GetCommandBlock("Offscreen");
        VkSubmitInfo submission = vk_submit_info_base;
        submission.commandBufferCount = 1;
        submission.pCommandBuffers = &block.Cmd;
        submission.waitSemaphoreCount = 1;
        submission.pWaitSemaphores = &acquiredBackBuffer->GetSemaphore("ImageAcquire");
        submission.signalSemaphoreCount = 1;
        submission.pSignalSemaphores = &acquiredBackBuffer->GetSemaphore("Offscreen");
        submission.pWaitDstStageMask = &OffscreenFlags;
        vkQueueSubmit(device->GraphicsQueue(), 1, &submission, block.Fence);
    }

    void ClusteredForward::recordComputePass(FrameData& frame) {
        auto& block = frame.GetCommandBlock("Compute");
        VkResult result = vkWaitForFences(device->vkHandle(), 1, &block.Fence, VK_TRUE, static_cast<uint64_t>(1.0e9)); VkAssert(result);
        vkResetFences(device->vkHandle(), 1, &block.Fence);
        if (!block.firstFrame) {
            vkResetCommandBuffer(block.Cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        }

        LightPositions->CopyToMapped(Lights.Positions.data(), sizeof(glm::vec4) * Lights.Positions.size(), 0);

        Barriers[0].srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        Barriers[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        Barriers[0].buffer = LightPositions->vkHandle();
        Barriers[0].size = LightPositions->Size();
        

        auto& cmd = block.Cmd;
        constexpr VkCommandBufferBeginInfo begin_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr };
        vkBeginCommandBuffer(cmd, &begin_info);
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, Barriers, 0, nullptr);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, Pipelines.ComputePipelines.Handles[NameIdxMap.at("LightGrids")]);
            VkDescriptorSet compute_sets[2]{ frameDescriptor->vkHandle(), texelBufferSet->vkHandle() };
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, Pipelines.ComputeLayout->vkHandle(), 0, 2, compute_sets, 0, nullptr);
            vkCmdDispatch(cmd, (ProgramState.NumLights - 1) / 32 + 1, 1, 1);
            Barriers[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT; Barriers[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            Barriers[0].buffer = lightData->Bounds->vkHandle(); Barriers[0].size = lightData->Bounds->Size(); 
            Barriers[1] = Barriers[0];
            Barriers[1].buffer = lightData->LightCounts->vkHandle(); Barriers[1].size = lightData->LightCounts->Size();
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 2, Barriers, 0, nullptr);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, Pipelines.ComputePipelines.Handles[NameIdxMap.at("GridOffsets")]);
            vkCmdDispatch(cmd, (ProgramState.TileCountX - 1) / 16 + 1, (ProgramState.TileCountY - 1) / 16 + 1, ProgramState.TileCountZ);
            Barriers[0].buffer = lightData->LightCountTotal->vkHandle(); Barriers[0].size = lightData->LightCountTotal->Size();
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, Barriers, 0, nullptr);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, Pipelines.ComputePipelines.Handles[NameIdxMap.at("LightList")]);
            vkCmdDispatch(cmd, (ProgramState.NumLights - 1) / 32 + 1, 1, 1);
        vkEndCommandBuffer(cmd);
        block.firstFrame = false;
    }

    void ClusteredForward::submitComputePass(FrameData& frame) {
        auto& block = frame.GetCommandBlock("Compute");
        VkSubmitInfo submission = vk_submit_info_base;
        submission.commandBufferCount = 1;
        submission.pCommandBuffers = &block.Cmd;
        submission.waitSemaphoreCount = 1;
        submission.pWaitSemaphores = &acquiredBackBuffer->GetSemaphore("Offscreen");
        submission.signalSemaphoreCount = 1;
        submission.pSignalSemaphores = &acquiredBackBuffer->GetSemaphore("Compute");
        submission.pWaitDstStageMask = &ComputeFlags;
        vkQueueSubmit(device->ComputeQueue(), 1, &submission, block.Fence);
    }

    void ClusteredForward::recordOnscreenPass(FrameData& frame) {
        auto& block = frame.GetCommandBlock("Onscreen");
        if (!block.firstFrame) {
            vkResetCommandBuffer(block.Cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        }

        ImGui::BeginMainMenuBar();
        ImGui::EndMainMenuBar();
        ImGui::Render();
        
        onscreenPass->UpdateBeginInfo(framebuffers[acquiredBackBuffer->GetImageIdx()]);
        auto& cmd = block.Cmd;
        constexpr VkCommandBufferBeginInfo begin_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr };
        vkBeginCommandBuffer(cmd, &begin_info);
            vkCmdBeginRenderPass(cmd, &onscreenPass->BeginInfo(), VK_SUBPASS_CONTENTS_INLINE);
            vkCmdSetViewport(cmd, 0, 1, &offscreenViewport);
            vkCmdSetScissor(cmd, 0, 1, &offscreenScissor);
            const VkDescriptorSet first_two_sets[2]{ texelBufferSet->vkHandle(), frameDescriptor->vkHandle() };
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines.Opaque.Layout->vkHandle(), 0, 2, first_two_sets, 0, nullptr);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines.Opaque.Pipeline->vkHandle());
            sponza->Render(DrawInfo{ cmd, Pipelines.Opaque.Layout->vkHandle(), true, true, 2 });
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines.Transparent.Pipeline->vkHandle());
            sponza->Render(DrawInfo{ cmd, Pipelines.Opaque.Layout->vkHandle(), false, true, 2 });
            gui->UpdateBuffers();
            gui->DrawFrame(cmd);
            renderParticles(cmd);
            vkCmdEndRenderPass(cmd);
            resetTexelBuffers(cmd);
        vkEndCommandBuffer(cmd);

        block.firstFrame = false;
    }

    void ClusteredForward::renderParticles(const VkCommandBuffer cmd) {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines.Particles.Pipeline->vkHandle());
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines.Depth.Layout->vkHandle(), 0, 1, &frameDescriptor->vkHandle(), 0, nullptr);
        constexpr static VkDeviceSize vert_offsets[2]{ 0, 0 };
        const VkBuffer buffers[2]{ Particles->Positions->vkHandle(), Particles->Colors->vkHandle() };
        vkCmdBindVertexBuffers(cmd, 0, 2, buffers, vert_offsets);
        vkCmdDraw(cmd, static_cast<uint32_t>(Lights.Positions.size()), 1, 0, 0);
    }

    void ClusteredForward::resetTexelBuffers(const VkCommandBuffer cmd) {
        lightData->ClearBuffers(cmd);
    }

    void ClusteredForward::submitOnscreenPass(FrameData& frame) {
        auto& block = frame.GetCommandBlock("Onscreen");
        VkSubmitInfo submission = vk_submit_info_base;
        submission.commandBufferCount = 1;
        submission.pCommandBuffers = &block.Cmd;
        submission.waitSemaphoreCount = 1;
        submission.pWaitSemaphores = &acquiredBackBuffer->GetSemaphore("Compute");
        submission.signalSemaphoreCount = 1;
        submission.pSignalSemaphores = &acquiredBackBuffer->GetSemaphore("Onscreen");
        submission.pWaitDstStageMask = &RenderFlags;
        vkQueueSubmit(device->GraphicsQueue(), 1, &submission, block.Fence);
    }

    void ClusteredForward::recordCommands() {
        constexpr static VkDeviceSize Offsets[1]{ 0 };
        CurrFrameDataIdx = acquiredBackBuffer->GetImageIdx();
        auto& frame_data = frames[CurrFrameDataIdx];

        recordPrecomputePass(frame_data);
        submitPrecomputePass(frame_data);
        recordComputePass(frame_data);
        submitComputePass(frame_data);
        recordOnscreenPass(frame_data);
        submitOnscreenPass(frame_data);

        
    }

    void ClusteredForward::presentBackBuffer() {
        VkPresentInfoKHR present_info = vk_present_info_base;
        present_info.pImageIndices = &acquiredBackBuffer->GetImageIdx();
        present_info.pWaitSemaphores = &acquiredBackBuffer->GetSemaphore("Onscreen");
        present_info.waitSemaphoreCount = 1;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain->vkHandle();
        VkResult result = vkQueuePresentKHR(device->GraphicsQueue(), &present_info); VkAssert(result);
        result = vkWaitForFences(device->vkHandle(), 1, &frames[CurrFrameDataIdx].GetCommandBlock("Onscreen").Fence, VK_TRUE, static_cast<uint64_t>(1.0e9)); VkAssert(result);
        vkResetFences(device->vkHandle(), 1, &frames[CurrFrameDataIdx].GetCommandBlock("Onscreen").Fence);
    }

    void ClusteredForward::createFrameData() {
        const uint32_t num_frames = swapchain->ImageCount();
        for (uint32_t i = 0; i < num_frames; ++i) {
            frames.emplace_back(FrameData{ device.get()});
            VkFenceCreateInfo create_info = vk_fence_create_info_base;
            create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            frames.back().AddCommandBlock("Compute", computePool->GetCmdBuffer(i), create_info);
            frames.back().AddCommandBlock("Offscreen", primaryPool->GetCmdBuffer(i * 2), create_info);
            frames.back().AddCommandBlock("Onscreen", primaryPool->GetCmdBuffer(i * 2 + 1), vk_fence_create_info_base);
        }  
    }

    void ClusteredForward::createBackbuffers() {
        const std::vector<std::string> semaphore_names{ std::string("ImageAcquire"), std::string("Offscreen"), std::string("Compute"), std::string("Onscreen") };
        for (size_t i = 0; i < swapchain->ImageCount(); ++i) {
            Backbuffers.emplace_back(std::make_unique<BackBuffer>(device.get(), swapchain->vkHandle(), semaphore_names));
        }
    }

    void ClusteredForward::createUBO() {
        ubo = std::make_unique<Buffer>(device.get());
        ubo->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sizeof(GlobalUBO));
    }

    void ClusteredForward::createLightBuffers() {
        lightData = std::make_unique<LightBuffers>(device.get());
        const uint32_t grid_size = ProgramState.TileCountX * ProgramState.TileCountY * ProgramState.TileCountZ;
        lightData->CreateBuffers(grid_size, ProgramState.MaxLights);
    }


    void ClusteredForward::createDepthPipeline() {

        Pipelines.Depth.Layout = std::make_unique<PipelineLayout>(device.get());
        const VkDescriptorSetLayout* set_layout = &frameDataLayout->vkHandle();
        Pipelines.Depth.Layout->Create(set_layout, 1);

        static const size_t name_hash = std::hash<std::string>()("depth-pipeline");
        Pipelines.Depth.Cache = std::make_unique<PipelineCache>(device.get(), name_hash);

        GraphicsPipelineInfo info;
        info.VertexInfo.vertexAttributeDescriptionCount = 1;
        info.VertexInfo.vertexBindingDescriptionCount = 1;
        // just use position input.
        static const VkVertexInputAttributeDescription attr = vertex_t::attributeDescriptions[0];
        static const VkVertexInputBindingDescription bind = vertex_t::bindingDescriptions[0];
        info.VertexInfo.pVertexAttributeDescriptions = &attr;
        info.VertexInfo.pVertexBindingDescriptions = &bind;

        info.DepthStencilInfo.depthTestEnable = VK_TRUE;
        info.DepthStencilInfo.depthWriteEnable = VK_TRUE;
        info.DepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

        info.ColorBlendInfo.attachmentCount = 0;
        info.ColorBlendInfo.pAttachments = nullptr;
        info.ColorBlendInfo.logicOpEnable = VK_FALSE;
        info.ColorBlendInfo.logicOp = VK_LOGIC_OP_CLEAR;

        info.DynamicStateInfo.dynamicStateCount = 2;
        static constexpr VkDynamicState States[2]{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        info.DynamicStateInfo.pDynamicStates = States;

        auto pipeline_info = info.GetPipelineCreateInfo();
        pipeline_info.stageCount = 1;
        VkPipelineShaderStageCreateInfo stage = shaders.at("Simple.vert")->PipelineInfo();
        const VkSpecializationInfo specializations{ static_cast<uint32_t>(SpecializationConstantMap.size()), SpecializationConstantMap.data(), sizeof(SpecializationConstants), &SpecializationConstants };
        stage.pSpecializationInfo = &specializations;
        pipeline_info.pStages = &shaders.at("Simple.vert")->PipelineInfo();
        pipeline_info.layout = Pipelines.Depth.Layout->vkHandle();
        pipeline_info.renderPass = computePass->vkHandle();
        pipeline_info.subpass = 0;

        Pipelines.Depth.Pipeline = std::make_unique<GraphicsPipeline>(device.get());
        Pipelines.Depth.Pipeline->Init(pipeline_info, Pipelines.Depth.Cache->vkHandle());
    }

    void ClusteredForward::createLightingPipelines() {

        Pipelines.LightingOpaque.Layout = std::make_unique<PipelineLayout>(device.get());
        const VkDescriptorSetLayout layouts[2]{ frameDataLayout->vkHandle(), texelBuffersLayout->vkHandle() };
        Pipelines.LightingOpaque.Layout->Create(layouts, 2);

        GraphicsPipelineInfo PipelineInfo;

        PipelineInfo.VertexInfo.pVertexAttributeDescriptions = &vertex_t::attributeDescriptions[0];
        PipelineInfo.VertexInfo.pVertexBindingDescriptions = &vertex_t::bindingDescriptions[0];
        PipelineInfo.VertexInfo.vertexAttributeDescriptionCount = 1;
        PipelineInfo.VertexInfo.vertexBindingDescriptionCount = 1;

        PipelineInfo.DepthStencilInfo.depthTestEnable = VK_TRUE;
        PipelineInfo.DepthStencilInfo.depthWriteEnable = VK_FALSE;

        PipelineInfo.ColorBlendInfo.attachmentCount = 0;
        PipelineInfo.ColorBlendInfo.pAttachments = nullptr;

        PipelineInfo.RasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        PipelineInfo.DepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

        constexpr static VkDynamicState states[2]{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        PipelineInfo.DynamicStateInfo.dynamicStateCount = 2;
        PipelineInfo.DynamicStateInfo.pDynamicStates = states;

        VkGraphicsPipelineCreateInfo create_info = PipelineInfo.GetPipelineCreateInfo();
        create_info.layout = Pipelines.LightingOpaque.Layout->vkHandle();
        VkPipelineShaderStageCreateInfo shader_infos[2]{ shaders.at("Light.vert")->PipelineInfo(), shaders.at("Light.frag")->PipelineInfo() };
        const uint16_t hash_id = static_cast<uint16_t>(std::hash<std::string>()("lighting-pipeline-opaque"));
        Pipelines.LightingOpaque.Cache = std::make_unique<PipelineCache>(device.get(), hash_id);
        create_info.subpass = 1;
        create_info.renderPass = computePass->vkHandle();
        create_info.stageCount = 2;
        create_info.pStages = shader_infos;
        const VkSpecializationInfo specializations{ static_cast<uint32_t>(SpecializationConstantMap.size()), SpecializationConstantMap.data(), sizeof(SpecializationConstants), &SpecializationConstants };
        shader_infos[0].pSpecializationInfo = &specializations;
        shader_infos[1].pSpecializationInfo = &specializations;
        Pipelines.LightingOpaque.Pipeline = std::make_unique<GraphicsPipeline>(device.get());
        Pipelines.LightingOpaque.Pipeline->Init(create_info, Pipelines.LightingOpaque.Cache->vkHandle());

        PipelineInfo.RasterizationInfo.cullMode = VK_CULL_MODE_NONE;

        Pipelines.LightingTransparent.Pipeline = std::make_unique<GraphicsPipeline>(device.get());
        Pipelines.LightingTransparent.Pipeline->Init(create_info, Pipelines.LightingOpaque.Cache->vkHandle());

    }

    void ClusteredForward::createMainPipelines() {

        Pipelines.Opaque.Layout = std::make_unique<PipelineLayout>(device.get());
        const VkDescriptorSetLayout set_layouts[3]{ texelBuffersLayout->vkHandle(), frameDataLayout->vkHandle(), sponza->GetMaterialSetLayout() };
        Pipelines.Opaque.Layout->Create(set_layouts, 3);

        const uint16_t hash_id = static_cast<uint16_t>(std::hash<std::string>()("main-pipeline"));
        Pipelines.Opaque.Cache = std::make_unique<PipelineCache>(device.get(), hash_id);

        VkPipelineColorBlendAttachmentState attachment_state = vk_pipeline_color_blend_attachment_info_base;

        GraphicsPipelineInfo PipelineInfo;

        PipelineInfo.VertexInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_t::attributeDescriptions.size());
        PipelineInfo.VertexInfo.pVertexAttributeDescriptions = vertex_t::attributeDescriptions.data();
        PipelineInfo.VertexInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_t::bindingDescriptions.size());
        PipelineInfo.VertexInfo.pVertexBindingDescriptions = vertex_t::bindingDescriptions.data();

        PipelineInfo.MultisampleInfo.sampleShadingEnable = VK_TRUE;
        PipelineInfo.MultisampleInfo.minSampleShading = 0.25f;
        PipelineInfo.MultisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_8_BIT;

        PipelineInfo.ColorBlendInfo.attachmentCount = 1;
        PipelineInfo.ColorBlendInfo.pAttachments = &attachment_state;
        PipelineInfo.DepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

        PipelineInfo.DynamicStateInfo.dynamicStateCount = 2;
        constexpr static VkDynamicState states[2]{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        PipelineInfo.DynamicStateInfo.pDynamicStates = states;

        VkGraphicsPipelineCreateInfo create_info = PipelineInfo.GetPipelineCreateInfo();
        create_info.subpass = 0;
        create_info.layout = Pipelines.Opaque.Layout->vkHandle();
        create_info.renderPass = onscreenPass->vkHandle();

        VkPipelineShaderStageCreateInfo stages[2]{ shaders.at("Clustered.vert")->PipelineInfo(), shaders.at("Clustered.frag")->PipelineInfo() };
        const VkSpecializationInfo specializations{ static_cast<uint32_t>(SpecializationConstantMap.size()), SpecializationConstantMap.data(), sizeof(SpecializationConstants), &SpecializationConstants };
        stages[0].pSpecializationInfo = &specializations;
        stages[1].pSpecializationInfo = &specializations;

        create_info.stageCount = 2;
        create_info.pStages = stages;
        attachment_state.blendEnable = VK_FALSE;

        Pipelines.Opaque.Pipeline = std::make_unique<GraphicsPipeline>(device.get());
        Pipelines.Opaque.Pipeline->Init(create_info, Pipelines.Opaque.Cache->vkHandle());

        PipelineInfo.RasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        PipelineInfo.DepthStencilInfo.depthWriteEnable = VK_FALSE;
        attachment_state.blendEnable = VK_TRUE;

        Pipelines.Transparent.Pipeline = std::make_unique<GraphicsPipeline>(device.get());
        Pipelines.Transparent.Pipeline->Init(create_info, Pipelines.Opaque.Cache->vkHandle());

    }

    void ClusteredForward::createParticlePipeline() {

        GraphicsPipelineInfo PipelineInfo;
        PipelineInfo.AssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        
        constexpr static std::array<VkVertexInputAttributeDescription, 2> attributes{
            VkVertexInputAttributeDescription{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
            VkVertexInputAttributeDescription{ 1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0 }
        };

        constexpr static std::array<VkVertexInputBindingDescription, 2> binding {
            VkVertexInputBindingDescription{ 0, sizeof(glm::vec4), VK_VERTEX_INPUT_RATE_VERTEX },
            VkVertexInputBindingDescription{ 1, sizeof(glm::vec4), VK_VERTEX_INPUT_RATE_VERTEX }
        };

        PipelineInfo.VertexInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(binding.size());
        PipelineInfo.VertexInfo.pVertexBindingDescriptions = binding.data();
        PipelineInfo.VertexInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
        PipelineInfo.VertexInfo.pVertexAttributeDescriptions = attributes.data();

        const VkDynamicState dyn_states[2]{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        PipelineInfo.DynamicStateInfo.dynamicStateCount = 2;
        PipelineInfo.DynamicStateInfo.pDynamicStates = dyn_states;

        PipelineInfo.MultisampleInfo.sampleShadingEnable = VK_TRUE;
        PipelineInfo.MultisampleInfo.minSampleShading = 0.25f;
        PipelineInfo.MultisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_8_BIT;

        auto create_info = PipelineInfo.GetPipelineCreateInfo();
        create_info.renderPass = onscreenPass->vkHandle();
        create_info.subpass = 0;
        create_info.layout = Pipelines.Depth.Layout->vkHandle();

        create_info.stageCount = 2;
        VkPipelineShaderStageCreateInfo stages[2]{ shaders.at("Particles.vert")->PipelineInfo(), shaders.at("Particles.frag")->PipelineInfo() };
        const VkSpecializationInfo specializations{ static_cast<uint32_t>(SpecializationConstantMap.size()), SpecializationConstantMap.data(), sizeof(SpecializationConstants), &SpecializationConstants };
        stages[0].pSpecializationInfo = &specializations;
        stages[1].pSpecializationInfo = &specializations;
        create_info.pStages = stages;

        const static size_t particle_hash = std::hash<std::string>()("particles-cache");
        Pipelines.Particles.Cache = std::make_unique<PipelineCache>(device.get(), particle_hash);
        Pipelines.Particles.Pipeline = std::make_unique<GraphicsPipeline>(device.get());
        Pipelines.Particles.Pipeline->Init(create_info, Pipelines.Particles.Cache->vkHandle());
    }

    void ClusteredForward::createComputePipelines() {
        Pipelines.ComputePipelines.Infos.fill(VkComputePipelineCreateInfo{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, nullptr, 0, VkPipelineShaderStageCreateInfo{}, VK_NULL_HANDLE, VK_NULL_HANDLE, -1 });
        Pipelines.ComputeLayout = std::make_unique<PipelineLayout>(device.get());
        const VkDescriptorSetLayout layouts[2]{ frameDataLayout->vkHandle(), texelBuffersLayout->vkHandle() };
        Pipelines.ComputeLayout->Create(layouts, 2);
        Pipelines.ComputePipelines.Infos[NameIdxMap.at("LightGrids")].layout = Pipelines.ComputeLayout->vkHandle();
        Pipelines.ComputePipelines.Infos[NameIdxMap.at("GridOffsets")].layout = Pipelines.ComputeLayout->vkHandle();
        Pipelines.ComputePipelines.Infos[NameIdxMap.at("LightList")].layout = Pipelines.ComputeLayout->vkHandle();
        const VkSpecializationInfo specializations{ static_cast<uint32_t>(SpecializationConstantMap.size()), SpecializationConstantMap.data(), sizeof(SpecializationConstants), &SpecializationConstants };
        Pipelines.ComputePipelines.Infos[NameIdxMap.at("LightGrids")].stage = shaders.at("LightGrid.comp")->PipelineInfo();
        Pipelines.ComputePipelines.Infos[NameIdxMap.at("LightGrids")].stage.pSpecializationInfo = &specializations;
        Pipelines.ComputePipelines.Infos[NameIdxMap.at("GridOffsets")].stage = shaders.at("GridOffsets.comp")->PipelineInfo();
        Pipelines.ComputePipelines.Infos[NameIdxMap.at("GridOffsets")].stage.pSpecializationInfo = &specializations;
        Pipelines.ComputePipelines.Infos[NameIdxMap.at("LightList")].stage = shaders.at("LightList.comp")->PipelineInfo();
        Pipelines.ComputePipelines.Infos[NameIdxMap.at("LightList")].stage.pSpecializationInfo = &specializations;
        Pipelines.ComputePipelines.Cache = std::make_unique<PipelineCache>(device.get(), static_cast<uint16_t>(std::hash<std::string>()("compute-pipeline-cache")));

        VkResult result = vkCreateComputePipelines(device->vkHandle(), Pipelines.ComputePipelines.Cache->vkHandle(),
            static_cast<uint32_t>(Pipelines.ComputePipelines.Infos.size()), Pipelines.ComputePipelines.Infos.data(),
            nullptr, Pipelines.ComputePipelines.Handles.data());
        VkAssert(result);
    }

    void ClusteredForward::createDescriptorPool() {
        descriptorPool = std::make_unique<DescriptorPool>(device.get(), swapchain->ImageCount() + 3);
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swapchain->ImageCount());
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, swapchain->ImageCount() * 2 + 7);
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2);
        descriptorPool->Create();
    }

    void ClusteredForward::createFrameDataSetLayout() {
        frameDataLayout = std::make_unique<DescriptorSetLayout>(device.get());
        constexpr static VkShaderStageFlags vfc_flags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
        constexpr static VkShaderStageFlags fc_flags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
        frameDataLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, vfc_flags, 0);
        frameDataLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, fc_flags, 1);
        frameDataLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, fc_flags, 2);

        frameDescriptor = std::make_unique<DescriptorSet>(device.get());
        frameDescriptor->AddDescriptorInfo(ubo->GetDescriptor(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);
        frameDescriptor->AddDescriptorInfo(LightPositions->GetDescriptor(), LightPositions->View(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1);
        frameDescriptor->AddDescriptorInfo(LightColors->GetDescriptor(), LightColors->View(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 2);
        frameDescriptor->Init(descriptorPool.get(), frameDataLayout.get());

    }

    void ClusteredForward::createTexelBufferSetLayout() {
        texelBuffersLayout = std::make_unique<DescriptorSetLayout>(device.get());
        constexpr static VkShaderStageFlags vfc_flags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
        constexpr static VkShaderStageFlags fc_flags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
        texelBuffersLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, fc_flags, 0);
        texelBuffersLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, fc_flags, 1);
        texelBuffersLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, fc_flags, 2);
        texelBuffersLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, fc_flags, 3);
        texelBuffersLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, fc_flags, 4);
        texelBuffersLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, fc_flags, 5);
        texelBuffersLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, fc_flags, 6);
    }

    void ClusteredForward::createTexelBufferDescriptorSet() {
        texelBufferSet = std::make_unique<DescriptorSet>(device.get());
        texelBufferSet->AddDescriptorInfo(lightData->Flags->GetDescriptor(), lightData->Flags->View(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 0);
        texelBufferSet->AddDescriptorInfo(lightData->Bounds->GetDescriptor(), lightData->Bounds->View(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1);
        texelBufferSet->AddDescriptorInfo(lightData->LightCounts->GetDescriptor(), lightData->LightCounts->View(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 2);
        texelBufferSet->AddDescriptorInfo(lightData->LightCountTotal->GetDescriptor(), lightData->LightCountTotal->View(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 3);
        texelBufferSet->AddDescriptorInfo(lightData->LightCountOffsets->GetDescriptor(), lightData->LightCountOffsets->View(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 4);
        texelBufferSet->AddDescriptorInfo(lightData->LightList->GetDescriptor(), lightData->LightList->View(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 5);
        texelBufferSet->AddDescriptorInfo(lightData->LightCountsCompare->GetDescriptor(), lightData->LightCountsCompare->View(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 6);
        texelBufferSet->Init(descriptorPool.get(), texelBuffersLayout.get());
    }

    void ClusteredForward::createComputeCmdPool() {
        VkCommandPoolCreateInfo pool_info = vk_command_pool_info_base;
        pool_info.queueFamilyIndex = device->QueueFamilyIndices.Compute;
        if (device->QueueFamilyIndices.Compute == device->QueueFamilyIndices.Graphics) {
            std::cerr << "Compute/Graphics queues are the same on current hardware.\n";
        }
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        computePool = std::make_unique<CommandPool>(device.get(), pool_info);
        computePool->AllocateCmdBuffers(swapchain->ImageCount(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    }

    void ClusteredForward::createPrimaryCmdPool() {
        VkCommandPoolCreateInfo pool_info = vk_command_pool_info_base;
        pool_info.queueFamilyIndex = device->QueueFamilyIndices.Graphics;
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        primaryPool = std::make_unique<CommandPool>(device.get(), pool_info);
        primaryPool->AllocateCmdBuffers(swapchain->ImageCount() * 2, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    }

    void ClusteredForward::createOnscreenAttachmentDescriptions() {
        onscreenDescr[0] = vk_attachment_description_base;
        onscreenDescr[0].format = swapchain->ColorFormat();
        onscreenDescr[0].samples = VK_SAMPLE_COUNT_8_BIT;
        onscreenDescr[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        onscreenDescr[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        onscreenDescr[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        onscreenDescr[1] = vk_attachment_description_base;
        onscreenDescr[1].format = swapchain->ColorFormat();
        onscreenDescr[1].samples = VK_SAMPLE_COUNT_1_BIT;
        onscreenDescr[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        onscreenDescr[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        onscreenDescr[2] = vk_attachment_description_base;
        onscreenDescr[2].format = device->FindDepthFormat();
        onscreenDescr[2].samples = VK_SAMPLE_COUNT_8_BIT;
        onscreenDescr[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        onscreenDescr[2].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    void ClusteredForward::createOnscreenAttachmentReferences() {
        onscreenReferences[0] = VkAttachmentReference{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        onscreenReferences[1] = VkAttachmentReference{ 1, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR };
        onscreenReferences[2] = VkAttachmentReference{ 2, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
    }

    void ClusteredForward::createOnscreenSubpassDescription() {
        onscreenSbDescr = vk_subpass_description_base;
        onscreenSbDescr.colorAttachmentCount = 1;
        onscreenSbDescr.pColorAttachments = &onscreenReferences[0];
        onscreenSbDescr.pResolveAttachments = &onscreenReferences[1];
        onscreenSbDescr.pDepthStencilAttachment = &onscreenReferences[2];
    }

    void ClusteredForward::createOnscreenSubpassDependencies() {
        onscreenDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        onscreenDependencies[0].dstSubpass = 0;
        onscreenDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        onscreenDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        onscreenDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        onscreenDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        onscreenDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        onscreenDependencies[1].srcSubpass = 0;
        onscreenDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        onscreenDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        onscreenDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        onscreenDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        onscreenDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        onscreenDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    }

    void ClusteredForward::createOnscreenPass() {
        createOnscreenAttachmentDescriptions();
        createOnscreenAttachmentReferences();
        createOnscreenSubpassDescription();
        createOnscreenSubpassDependencies();

        VkRenderPassCreateInfo create_info = vk_render_pass_create_info_base;
        create_info.attachmentCount = static_cast<uint32_t>(onscreenDescr.size());
        create_info.pAttachments = onscreenDescr.data();
        create_info.subpassCount = 1;
        create_info.pSubpasses = &onscreenSbDescr;
        create_info.dependencyCount = static_cast<uint32_t>(onscreenDependencies.size());
        create_info.pDependencies = onscreenDependencies.data();
        
        onscreenPass = std::make_unique<Renderpass>(device.get(), create_info);
        onscreenPass->SetupBeginInfo(OnscreenClearValues.data(), OnscreenClearValues.size(), swapchain->Extent());
    }

    void ClusteredForward::createComputeAttachmentDescriptions() {
        computeAttachmentDescr[0] = vk_attachment_description_base;
        computeAttachmentDescr[0] = VkAttachmentDescription{
            0,
            device->FindDepthFormat(),
            VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_CLEAR,
            VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };
    }

    void ClusteredForward::createComputeSubpassDescriptions() {
        computeRef = VkAttachmentReference{ 0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

        computeSbDescr[0] = VkSubpassDescription{
            0, VK_PIPELINE_BIND_POINT_GRAPHICS,
            0, nullptr, 0, nullptr,
            nullptr, &computeRef,
            0, nullptr
        };

        computeSbDescr[1] = computeSbDescr[0];
    }

    void ClusteredForward::createComputeSubpassDependencies() {

        computeDependencies[0] = VkSubpassDependency{
            VK_SUBPASS_EXTERNAL,
            0,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
            VK_ACCESS_MEMORY_WRITE_BIT,
            VK_ACCESS_UNIFORM_READ_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        };

        computeDependencies[1] = VkSubpassDependency{
            0,
            1,
            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        };

        computeDependencies[2] = VkSubpassDependency{
            1,
            VK_SUBPASS_EXTERNAL,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_ACCESS_SHADER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_DEPENDENCY_BY_REGION_BIT
        };

    }

    void ClusteredForward::createComputePass() {
        createComputeAttachmentDescriptions();
        createComputeSubpassDescriptions();
        createComputeSubpassDependencies();

        VkRenderPassCreateInfo create_info = vk_render_pass_create_info_base;
        create_info.attachmentCount = static_cast<uint32_t>(computeAttachmentDescr.size());
        create_info.pAttachments = computeAttachmentDescr.data();
        create_info.subpassCount = static_cast<uint32_t>(computeSbDescr.size());
        create_info.pSubpasses = computeSbDescr.data();
        create_info.dependencyCount = static_cast<uint32_t>(computeDependencies.size());
        create_info.pDependencies = computeDependencies.data();

        computePass = std::make_unique<Renderpass>(device.get(), create_info);
        computePass->SetupBeginInfo(OffscreenClearValues.data(), OffscreenClearValues.size(), swapchain->Extent());
    }

    void ClusteredForward::createShaders() {
        const std::string prefix("../rsrc/shaders/clustered_forward/");
        shaders["Simple.vert"] = std::make_unique<ShaderModule>(device.get(), prefix + "Simple.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        shaders["Light.vert"] = std::make_unique<ShaderModule>(device.get(), prefix + "Light.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        shaders["Light.frag"] = std::make_unique<ShaderModule>(device.get(), prefix + "Light.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
        shaders["Clustered.vert"] = std::make_unique<ShaderModule>(device.get(), prefix + "Clustered.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        shaders["Clustered.frag"] = std::make_unique<ShaderModule>(device.get(), prefix + "Clustered.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
        shaders["Particles.vert"] = std::make_unique<ShaderModule>(device.get(), prefix + "Particles.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
        shaders["Particles.frag"] = std::make_unique<ShaderModule>(device.get(), prefix + "Particles.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
        shaders["GridOffsets.comp"] = std::make_unique<ShaderModule>(device.get(), prefix + "GridOffsets.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
        shaders["LightGrid.comp"] = std::make_unique<ShaderModule>(device.get(), prefix + "LightGrid.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
        shaders["LightList.comp"] = std::make_unique<ShaderModule>(device.get(), prefix + "LightList.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
    }

    void ClusteredForward::createOffscreenRenderTarget() {
        offscreenRenderTarget = std::make_unique<Image>(device.get());
        VkImageCreateInfo create_info = vk_image_create_info_base;
        create_info.extent = VkExtent3D{ swapchain->Extent().width, swapchain->Extent().height, 1 };
        create_info.arrayLayers = 1;
        create_info.mipLevels = 1;
        create_info.imageType = VK_IMAGE_TYPE_2D;
        create_info.format = device->FindDepthFormat();
        create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        offscreenRenderTarget->Create(create_info);
        offscreenRenderTarget->CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    void ClusteredForward::createOffscreenFramebuffer() {
        VkFramebufferCreateInfo create_info = vk_framebuffer_create_info_base;
        create_info.attachmentCount = 1;
        create_info.pAttachments = &offscreenRenderTarget->View();
        create_info.renderPass = computePass->vkHandle();
        create_info.width = swapchain->Extent().width;
        create_info.height = swapchain->Extent().height;
        create_info.layers = 1;
        offscreenFramebuffer = std::make_unique<Framebuffer>(device.get(), create_info);
    }

    void ClusteredForward::createRenderTargets() {
        msaa = std::make_unique<Multisampling>(device.get(), swapchain.get(), VK_SAMPLE_COUNT_8_BIT);
        depthStencil = std::make_unique<DepthStencil>(device.get(), VkExtent3D{ swapchain->Extent().width, swapchain->Extent().height, 1 });

        std::array<VkImageView, 3> attachments{ msaa->ColorBufferMS->View(), VK_NULL_HANDLE, msaa->DepthBufferMS->View() };
        VkFramebufferCreateInfo create_info = vk_framebuffer_create_info_base;
        create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        create_info.pAttachments = attachments.data();
        create_info.renderPass = onscreenPass->vkHandle();
        create_info.width = swapchain->Extent().width;
        create_info.height = swapchain->Extent().height;
        create_info.layers = 1;

        for (uint32_t i = 0; i < swapchain->ImageCount(); ++i) {
            attachments[1] = swapchain->ImageView(i);
            VkFramebuffer fbuff;
            VkResult result = vkCreateFramebuffer(device->vkHandle(), &create_info, nullptr, &fbuff);
            VkAssert(result);
            framebuffers.push_back(fbuff);
        }
    }

    void ClusteredForward::createVulkanLightData() {
        constexpr static VkMemoryPropertyFlags mem_flags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        VkBufferCreateInfo create_info = vk_buffer_create_info_base;
        uint32_t queue_indices[2]{ 0, 0 };

        LightPositions = std::make_unique<Buffer>(device.get());
        LightColors = std::make_unique<Buffer>(device.get());

        if (device->QueueFamilyIndices.Compute != device->QueueFamilyIndices.Graphics) {
            create_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
            queue_indices[0] = device->QueueFamilyIndices.Graphics;
            queue_indices[1] = device->QueueFamilyIndices.Compute;
            create_info.pQueueFamilyIndices = queue_indices;
            create_info.queueFamilyIndexCount = 2;
        }
        else {
            create_info.pQueueFamilyIndices = nullptr;
            create_info.queueFamilyIndexCount = 0;
        }

        create_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
        create_info.size = sizeof(glm::vec4) * ProgramState.MaxLights;
        LightPositions->CreateBuffer(create_info, mem_flags);
        LightPositions->CopyToMapped(Lights.Positions.data(), Lights.Positions.size() * sizeof(glm::vec4), 0);
        LightPositions->CreateView(VK_FORMAT_R32G32B32A32_SFLOAT, LightPositions->Size(), 0);
        create_info.size = sizeof(uint8_t) * 4 * ProgramState.MaxLights;
        LightColors->CreateBuffer(create_info, mem_flags);
        LightColors->CopyToMapped(Lights.Colors.data(), Lights.Colors.size() * sizeof(glm::u8vec4), 0);
        LightColors->CreateView(VK_FORMAT_R8G8B8A8_UNORM, LightColors->Size(), 0);
    }


    light_particles_t::light_particles_t(const vpr::Device * dvc, const lights_soa_t * lights) : Positions(std::make_unique<Buffer>(dvc)), Colors(std::make_unique<Buffer>(dvc)), Device(dvc), lightsData(lights) {
        Positions->CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sizeof(glm::vec4) * 4096);
        Colors->CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sizeof(glm::vec4) * 4096);
        update();
    }

    void light_particles_t::update() {
        Positions->CopyToMapped(lightsData->Positions.data(), sizeof(glm::vec4) * lightsData->Positions.size(), 0);
        std::vector<glm::vec4> float_colors;
        for (auto& color : lightsData->Colors) {
            float_colors.emplace_back(glm::vec4{ static_cast<float>(color.x), static_cast<float>(color.y), static_cast<float>(color.z), static_cast<float>(color.w) });
        }
        Colors->CopyToMapped(float_colors.data(), sizeof(glm::vec4) * float_colors.size(), 0);
    }

}

int main(int argc, char* argv[]) {
    using namespace vpsk;
    using namespace vpr;
    BaseScene::SceneConfiguration.EnableMouseLocking = true;
    BaseScene::SceneConfiguration.MovementSpeed = 10.0f;
    ClusteredForward fwd("../rsrc/crytekSponza/sponza.obj");
    fwd.RenderLoop();
    return 0;
}