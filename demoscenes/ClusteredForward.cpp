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
#include <memory>
#include <map>
#include <random>
#include <deque>

#include "util/easylogging++.h"
INITIALIZE_EASYLOGGINGPP

namespace vpsk {

    // https://mynameismjp.wordpress.com/2016/03/25/bindless-texturing-for-deferred-rendering-and-decals/

    using namespace vpr;

    struct program_state_t {
        uint32_t ResolutionX = 1440;
        uint32_t ResolutionY = 900;
        uint32_t MinLights = 1024;
        uint32_t MaxLights = 4096;
        uint32_t NumLights = 2048;
        bool GenerateLights = false;
        uint32_t TileWidth = 64;
        uint32_t TileHeight = 64;
        uint32_t TileCountX = 0;
        uint32_t TileCountY = 0;
        uint32_t TileCountZ = 256;
    } ProgramState;

    struct lights_soa_t {
        void update() {

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
                return std::fmaxf(std::numeric_limits<float>::epsilon(), std::fminf(3.14159f - std::numeric_limits<float>::epsilon(), y));
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

    class light_buffers_t {
        light_buffers_t(const light_buffers_t&) = delete;
        light_buffers_t& operator=(const light_buffers_t&) = delete;
    public:
        light_buffers_t() = default;
        light_buffers_t(light_buffers_t&& other) noexcept;
        light_buffers_t& operator=(light_buffers_t&& other) noexcept;
        light_buffers_t(const Device* dvc);
        void CreateBuffers();
        std::unique_ptr<Buffer> Flags;
        std::unique_ptr<Buffer> Bounds;
        std::unique_ptr<Buffer> LightCounts;
        std::unique_ptr<Buffer> LightCountTotal;
        std::unique_ptr<Buffer> LightCountOffsets;
        std::unique_ptr<Buffer> LightList;
        std::unique_ptr<Buffer> LightCountsCompare;
    };

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
        uint32_t NumLights;
    } GlobalUBO;

    struct specialization_constants_t {
        uint32_t ResolutionX = 1440;
        uint32_t ResolutionY = 900;
        uint32_t LightListMax = 2048;
        uint32_t TileWidth = 64;
        uint32_t TileHeight = 64;
        uint32_t TileCountX = 0;
        uint32_t TileCountY = 0;
        uint32_t TileCountZ = 256;
        uint32_t NearPlane = 0.1f;
        uint32_t FarPlane = 3000.0f;
        uint32_t AmbientGlobal = 0.20f;
    } SpecializationConstants;

    constexpr static std::array<VkSpecializationMapEntry, 11> SpecializationConstantMap {
        VkSpecializationMapEntry{ 0, 0, sizeof(uint32_t) },
        VkSpecializationMapEntry{ 1, sizeof(uint32_t), sizeof(uint32_t) },
        VkSpecializationMapEntry{ 2, sizeof(uint32_t) * 2, sizeof(uint32_t) },
        VkSpecializationMapEntry{ 3, sizeof(uint32_t) * 3, sizeof(uint32_t) },
        VkSpecializationMapEntry{ 4, sizeof(uint32_t) * 4, sizeof(uint32_t) },
        VkSpecializationMapEntry{ 5, sizeof(uint32_t) * 5, sizeof(uint32_t) },
        VkSpecializationMapEntry{ 6, sizeof(uint32_t) * 6, sizeof(uint32_t) },
        VkSpecializationMapEntry{ 7, sizeof(uint32_t) * 7, sizeof(uint32_t) },
        VkSpecializationMapEntry{ 8, sizeof(uint32_t) * 8, sizeof(float) },
        VkSpecializationMapEntry{ 9, sizeof(uint32_t) * 8 + sizeof(float), sizeof(float) },
        VkSpecializationMapEntry{10, sizeof(uint32_t) * 8 + 2 * sizeof(float), sizeof(float) }
    };

    struct frame_data_t {

        frame_data_t(const Device* dvc, uint32_t idx);

        struct cmd_buffer_block_t {
            VkCommandBuffer Cmd = VK_NULL_HANDLE;
            VkFence Fence = VK_NULL_HANDLE;
            bool firstFrame = true;
        };

        cmd_buffer_block_t OffscreenCmd;
        cmd_buffer_block_t ComputeCmd;
        cmd_buffer_block_t RenderCmd;

        const Device* device;
        std::unique_ptr<Buffer> LightPositions;
        std::unique_ptr<Buffer> LightColors;
        std::unique_ptr<DescriptorSet> descriptor;
        uint32_t idx;
    };

    struct g_pipeline_items_t {
        std::unique_ptr<GraphicsPipeline> Pipeline;
        std::unique_ptr<PipelineLayout> Layout;
        std::unique_ptr<PipelineCache> Cache;
    };

    static const std::map<const std::string, const size_t> NameIdxMap { 
            { "LightGrids", 0 }, { "GridOffsets", 1 }, { "LightList", 2 }
    };

    struct compute_pipelines_t {
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
        std::unique_ptr<PipelineLayout> ComputeLayout;
    };

    struct backbuffer_data_t {
        backbuffer_data_t();
        backbuffer_data_t(const backbuffer_data_t&) = delete;
        backbuffer_data_t& operator=(const backbuffer_data_t&) = delete;
        backbuffer_data_t(backbuffer_data_t&& other) noexcept;
        backbuffer_data_t& operator=(backbuffer_data_t&& other) noexcept;
        backbuffer_data_t(const Device* dvc);
        ~backbuffer_data_t();
        uint32_t idx = 0;
        VkSemaphore ImageAcquire;
        VkSemaphore PreCompute;
        VkSemaphore Compute;
        VkSemaphore Render;
        VkFence QueueSubmit;
        const Device* device;
    };

    class ClusteredForward : public BaseScene {
    public:

        void UpdateUBO();

        ClusteredForward(const std::string& obj_file);
        void RecordCommands() final;
        void RenderLoop() final;

    private:

        void WindowResized() final {}
        void RecreateObjects() final {}
        void endFrame(const size_t& curr_idx) final {}

        void updateUniforms();

        void acquireBackBuffer();
        void recordPrecomputePass(frame_data_t& frame);
        void submitPrecomputePass(frame_data_t & frame);
        void recordComputePass(frame_data_t & frame);
        void submitComputePass(frame_data_t & frame);
        void recordOnscreenPass(frame_data_t & frame);
        void submitOnscreenPass(frame_data_t & frame);
        void resetTexelBuffers(const VkCommandBuffer cmd);
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
        
        light_buffers_t LightBuffers;
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
        std::vector<frame_data_t> FrameData;
        std::deque<backbuffer_data_t> Backbuffers;
        backbuffer_data_t acquiredBackBuffer;
        VkSubmitInfo OffscreenSubmit, ComputeSubmit, RenderSubmit;
        std::map<std::string, std::unique_ptr<ShaderModule>> shaders;
        VkBufferMemoryBarrier Barriers[2]{ vk_buffer_memory_barrier_info_base, vk_buffer_memory_barrier_info_base };
        std::unique_ptr<Buffer> ubo;
        std::unique_ptr<ObjModel> sponza;
        std::unique_ptr<TexturePool> texturePool;

        VkViewport offscreenViewport, onscreenViewport;
        VkRect2D offscreenScissor, onscreenScissor;

        constexpr static VkPipelineStageFlags OffscreenFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        constexpr static VkPipelineStageFlags ComputeFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        constexpr static VkPipelineStageFlags RenderFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

        std::vector<VkFramebuffer> framebuffers;
        
    };

    void ClusteredForward::UpdateUBO() {
        ubo->CopyToMapped(&GlobalUBO, sizeof(GlobalUBO), 0);
    }
    
    light_buffers_t::light_buffers_t(light_buffers_t && other) noexcept : Flags(std::move(other.Flags)), Bounds(std::move(other.Bounds)), LightCounts(std::move(other.LightCounts)),
        LightCountTotal(std::move(other.LightCountTotal)), LightCountOffsets(std::move(other.LightCountOffsets)), LightList(std::move(other.LightList)), LightCountsCompare(std::move(other.LightCountsCompare)) {}

    light_buffers_t & light_buffers_t::operator=(light_buffers_t && other) noexcept {
        Flags = std::move(other.Flags);
        Bounds = std::move(other.Bounds);
        LightCounts = std::move(other.LightCounts);
        LightCountTotal = std::move(other.LightCountTotal);
        LightCountOffsets = std::move(other.LightCountOffsets);
        LightList = std::move(other.LightList);
        LightCountsCompare = std::move(other.LightCountsCompare);
        return *this;
    }

    light_buffers_t::light_buffers_t(const Device* dvc) : Flags(std::make_unique<Buffer>(dvc)), Bounds(std::make_unique<Buffer>(dvc)),
        LightCounts(std::make_unique<Buffer>(dvc)), LightCountTotal(std::make_unique<Buffer>(dvc)), LightCountOffsets(std::make_unique<Buffer>(dvc)),
        LightList(std::make_unique<Buffer>(dvc)), LightCountsCompare(std::make_unique<Buffer>(dvc)) {}

    void light_buffers_t::CreateBuffers() {
        const uint32_t max_grid_count = ((ProgramState.ResolutionX - 1) / (ProgramState.TileWidth + 1)) 
            * ((ProgramState.ResolutionY - 1) / (ProgramState.TileWidth + 1)) * ProgramState.TileCountZ;
        constexpr VkBufferUsageFlags buffer_flags = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        Flags->CreateBuffer(buffer_flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(uint8_t) * max_grid_count);
        Bounds->CreateBuffer(buffer_flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(uint32_t) * 6 * ProgramState.MaxLights);
        LightCounts->CreateBuffer(buffer_flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, max_grid_count * sizeof(uint32_t));
        LightCountTotal->CreateBuffer(buffer_flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(uint32_t));
        LightCountOffsets->CreateBuffer(buffer_flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, max_grid_count * sizeof(uint32_t));
        LightList->CreateBuffer(buffer_flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(uint32_t) * 1024 * 1024);
        LightCountsCompare->CreateBuffer(buffer_flags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(uint32_t) * max_grid_count);
    }

    backbuffer_data_t::backbuffer_data_t() : idx(0), ImageAcquire(VK_NULL_HANDLE), PreCompute(VK_NULL_HANDLE), Compute(VK_NULL_HANDLE), Render(VK_NULL_HANDLE), QueueSubmit(VK_NULL_HANDLE), device(nullptr) {}

    backbuffer_data_t::backbuffer_data_t(backbuffer_data_t && other) noexcept : idx(std::move(other.idx)), ImageAcquire(std::move(other.ImageAcquire)), PreCompute(std::move(other.PreCompute)),
        Compute(std::move(other.Compute)), Render(std::move(other.Render)), QueueSubmit(std::move(other.QueueSubmit)), device(std::move(other.device)) {
        other.ImageAcquire = VK_NULL_HANDLE;
        other.PreCompute = VK_NULL_HANDLE;
        other.Compute = VK_NULL_HANDLE;
        other.Render = VK_NULL_HANDLE;
        other.QueueSubmit = VK_NULL_HANDLE;
    }

    backbuffer_data_t& backbuffer_data_t::operator=(backbuffer_data_t&& other) noexcept {
        idx = std::move(other.idx);
        ImageAcquire = std::move(other.ImageAcquire);
        other.ImageAcquire = VK_NULL_HANDLE;
        PreCompute = std::move(other.PreCompute);
        other.PreCompute = VK_NULL_HANDLE;
        Compute = std::move(other.Compute);
        other.Compute = VK_NULL_HANDLE;
        Render = std::move(other.Render);
        other.Render = VK_NULL_HANDLE;
        QueueSubmit = std::move(other.QueueSubmit);
        other.QueueSubmit = VK_NULL_HANDLE;
        device = std::move(other.device);
        return *this;
    }

    backbuffer_data_t::backbuffer_data_t(const Device * dvc) : device(dvc) {
        VkResult result = VK_SUCCESS;
        result = vkCreateSemaphore(device->vkHandle(), &vk_semaphore_create_info_base, nullptr, &Render); VkAssert(result);
        result = vkCreateSemaphore(device->vkHandle(), &vk_semaphore_create_info_base, nullptr, &Compute); VkAssert(result);
        result = vkCreateSemaphore(device->vkHandle(), &vk_semaphore_create_info_base, nullptr, &PreCompute); VkAssert(result);
        result = vkCreateSemaphore(device->vkHandle(), &vk_semaphore_create_info_base, nullptr, &ImageAcquire); VkAssert(result);
        VkFenceCreateInfo fence_info = vk_fence_create_info_base;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        result = vkCreateFence(device->vkHandle(), &fence_info, nullptr, &QueueSubmit); VkAssert(result);
    }

    backbuffer_data_t::~backbuffer_data_t() {
        if (QueueSubmit != VK_NULL_HANDLE) {
            vkDestroyFence(device->vkHandle(), QueueSubmit, nullptr);
        }
        if (ImageAcquire != VK_NULL_HANDLE) {
            vkDestroySemaphore(device->vkHandle(), ImageAcquire, nullptr);
        }
        if (PreCompute != VK_NULL_HANDLE) {
            vkDestroySemaphore(device->vkHandle(), PreCompute, nullptr);
        }
        if (Compute != VK_NULL_HANDLE) {
            vkDestroySemaphore(device->vkHandle(), Compute, nullptr);
        }
        if (Render != VK_NULL_HANDLE) {
            vkDestroySemaphore(device->vkHandle(), Render, nullptr);
        }
    }

    frame_data_t::frame_data_t(const Device* dvc, const uint32_t _idx) : idx(_idx), LightPositions(std::make_unique<Buffer>(dvc)),
        LightColors(std::make_unique<Buffer>(dvc)), device(dvc) {
        constexpr static VkMemoryPropertyFlags mem_flags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        VkBufferCreateInfo create_info = vk_buffer_create_info_base;
        uint32_t queue_indices[2]{ 0, 0 };

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

        VkFenceCreateInfo fence_info = vk_fence_create_info_base;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VkResult result = vkCreateFence(device->vkHandle(), &fence_info, nullptr, &OffscreenCmd.Fence); VkAssert(result);
        result = vkCreateFence(device->vkHandle(), &fence_info, nullptr, &ComputeCmd.Fence); VkAssert(result);
        result = vkCreateFence(device->vkHandle(), &fence_info, nullptr, &RenderCmd.Fence); VkAssert(result);
    }


    ClusteredForward::ClusteredForward(const std::string & obj_file) : BaseScene(1440, 900) {
        ProgramState.TileCountX = (ProgramState.ResolutionX - 1) / ProgramState.TileWidth + 1;
        ProgramState.TileCountY = (ProgramState.ResolutionY - 1) / ProgramState.TileHeight + 1;
        texturePool = std::make_unique<TexturePool>(device.get(), transferPool.get());
        sponza = std::make_unique<ObjModel>(device.get(), texturePool.get());
        sponza->LoadModelFromFile(obj_file, transferPool.get());
        GenerateLights(sponza->GetAABB());
        createBackbuffers();
        createShaders();
        createUBO();
        createLightBuffers();
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
        createOffscreenRenderTarget();
        createOffscreenFramebuffer();
        createComputePipelines();
        createRenderTargets();
    }

    void ClusteredForward::RecordCommands() {

    }

    void ClusteredForward::RenderLoop() {

        offscreenViewport.width = swapchain->Extent().width;
        offscreenViewport.height = swapchain->Extent().height;
        offscreenViewport.minDepth = 0.0f;
        offscreenViewport.maxDepth = 3000.0f;
        offscreenViewport.x = 0;
        offscreenViewport.y = 0;
        offscreenScissor.offset.x = 0;
        offscreenScissor.offset.y = 0;
        offscreenScissor.extent.width = swapchain->Extent().width;
        offscreenScissor.extent.height = swapchain->Extent().height;
        const glm::vec3 eye = sponza->GetAABB().Center() + glm::vec3(0.0f, 20.0f, 50.0f);
        glm::mat4 view = glm::lookAt(eye, sponza->GetAABB().Center(), glm::vec3(0.0f, 1.0f, 0.0f));
        GlobalUBO.view = view;
        GlobalUBO.model = sponza->GetModelMatrix();
        GlobalUBO.normal = glm::transpose(glm::inverse(GlobalUBO.model));


        Buffer::DestroyStagingResources(device.get());

        while (!glfwWindowShouldClose(window->glfwWindow())) {
            glfwPollEvents();
            if (ShouldResize.exchange(false)) {
                RecreateSwapchain();
            }

            updateUniforms();

            limitFrame();
            UpdateMouseActions();
            //if (SceneConfiguration.EnableGUI) {
             //   gui->NewFrame(window->glfwWindow());
            //}

            UpdateMovement(BaseScene::SceneConfiguration.FrameTimeMs / 1000.0f);

            acquireBackBuffer();
            recordCommands();
            presentBackBuffer();

        }
    }

    void ClusteredForward::updateUniforms() {
        //GlobalUBO.view = GetViewMatrix();
        GlobalUBO.projection = GetProjectionMatrix();
        GlobalUBO.viewPosition = glm::vec4(GetCameraPosition(), 1.0f);
        GlobalUBO.NumLights = ProgramState.NumLights;
    }

    void ClusteredForward::acquireBackBuffer() {
        static bool first_frame = true;
        auto& curr = Backbuffers.front();

        VkResult result = vkWaitForFences(device->vkHandle(), 1, &curr.QueueSubmit, VK_TRUE, static_cast<uint64_t>(1e9)); VkAssert(result);
        vkResetFences(device->vkHandle(), 1, &curr.QueueSubmit);

        vkAcquireNextImageKHR(device->vkHandle(), swapchain->vkHandle(), 1e9, curr.ImageAcquire, VK_NULL_HANDLE, &curr.idx);
        if (!first_frame) {
            Backbuffers.push_back(std::move(acquiredBackBuffer));
        }
        acquiredBackBuffer = std::move(curr);
        Backbuffers.pop_front();
        first_frame = false;
    }

    void ClusteredForward::recordPrecomputePass(frame_data_t& frame) {
        VkResult result = vkWaitForFences(device->vkHandle(), 1, &frame.OffscreenCmd.Fence, VK_TRUE, static_cast<uint64_t>(1.0e9)); VkAssert(result);
        vkResetFences(device->vkHandle(), 1, &frame.OffscreenCmd.Fence);
        if (!frame.OffscreenCmd.firstFrame) {
            vkResetCommandBuffer(frame.OffscreenCmd.Cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        }

        Barriers[0].srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        Barriers[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        Barriers[0].buffer = ubo->vkHandle();
        Barriers[0].size = ubo->Size();

        UpdateUBO();
        //frame.LightPositions->CopyToMapped(Lights.Positions.data(), Lights.Positions.size() * sizeof(glm::vec4), 0);
        offscreenViewport.maxDepth = 1.0f;
        computePass->UpdateBeginInfo(offscreenFramebuffer->vkHandle());
        auto& cmd = frame.OffscreenCmd.Cmd;
        constexpr VkCommandBufferBeginInfo begin_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr };
        vkBeginCommandBuffer(cmd, &begin_info);
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &Barriers[0], 0, nullptr);
        vkCmdBeginRenderPass(cmd, &computePass->BeginInfo(), VK_SUBPASS_CONTENTS_INLINE);
            vkCmdSetViewport(cmd, 0, 1, &offscreenViewport);
            vkCmdSetScissor(cmd, 0, 1, &offscreenScissor);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines.Depth.Layout->vkHandle(), 0, 1, &frame.descriptor->vkHandle(), 0, nullptr);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines.Depth.Pipeline->vkHandle());
            sponza->Render(DrawInfo{ cmd, Pipelines.Depth.Layout->vkHandle(), true, false, 0 });
        vkCmdNextSubpass(cmd, VK_SUBPASS_CONTENTS_INLINE);
            const VkDescriptorSet descriptors[2]{ frame.descriptor->vkHandle(), texelBufferSet->vkHandle() };
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines.LightingOpaque.Layout->vkHandle(), 0, 2, descriptors, 0, nullptr);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines.LightingOpaque.Pipeline->vkHandle());
            sponza->Render(DrawInfo{ cmd, Pipelines.LightingOpaque.Layout->vkHandle(), true, false, 2 });
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines.LightingTransparent.Pipeline->vkHandle());
            sponza->Render(DrawInfo{ cmd, Pipelines.LightingOpaque.Layout->vkHandle(), false, false, 2 });
        vkCmdEndRenderPass(cmd);
        vkEndCommandBuffer(cmd);

        frame.OffscreenCmd.firstFrame = false;
    }

    void ClusteredForward::submitPrecomputePass(frame_data_t& frame) {
        VkSubmitInfo submission = vk_submit_info_base;
        submission.commandBufferCount = 1;
        submission.pCommandBuffers = &frame.OffscreenCmd.Cmd;
        submission.waitSemaphoreCount = 1;
        submission.pWaitSemaphores = &acquiredBackBuffer.ImageAcquire;
        submission.signalSemaphoreCount = 1;
        submission.pSignalSemaphores = &acquiredBackBuffer.PreCompute;
        submission.pWaitDstStageMask = &OffscreenFlags;
        vkQueueSubmit(device->GraphicsQueue(), 1, &submission, frame.OffscreenCmd.Fence);
    }

    void ClusteredForward::recordComputePass(frame_data_t& frame) {
        VkResult result = vkWaitForFences(device->vkHandle(), 1, &frame.ComputeCmd.Fence, VK_TRUE, static_cast<uint64_t>(1.0e9)); VkAssert(result);
        vkResetFences(device->vkHandle(), 1, &frame.ComputeCmd.Fence);
        if (!frame.ComputeCmd.firstFrame) {
            vkResetCommandBuffer(frame.ComputeCmd.Cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        }
        Lights.update();

        Barriers[0].srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        Barriers[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        Barriers[0].buffer = frame.LightPositions->vkHandle();
        Barriers[0].size = frame.LightPositions->Size();

        auto& cmd = frame.ComputeCmd.Cmd;
        constexpr VkCommandBufferBeginInfo begin_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr };
        vkBeginCommandBuffer(cmd, &begin_info);
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, Barriers, 0, nullptr);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, Pipelines.ComputePipelines.Handles[NameIdxMap.at("LightGrids")]);
            VkDescriptorSet compute_sets[2]{ frame.descriptor->vkHandle(), texelBufferSet->vkHandle() };
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, Pipelines.ComputeLayout->vkHandle(), 0, 2, compute_sets, 0, nullptr);
            vkCmdDispatch(cmd, (ProgramState.NumLights - 1) / 32 + 1, 1, 1);
            Barriers[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT; Barriers[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            Barriers[0].buffer = LightBuffers.Bounds->vkHandle(); Barriers[0].size = LightBuffers.Bounds->Size(); Barriers[1] = Barriers[0];
            Barriers[1].buffer = LightBuffers.LightCounts->vkHandle(); Barriers[1].size = LightBuffers.LightCounts->Size();
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 2, Barriers, 0, nullptr);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, Pipelines.ComputePipelines.Handles[NameIdxMap.at("GridOffsets")]);
            vkCmdDispatch(cmd, (ProgramState.TileCountX - 1) / 16 + 1, (ProgramState.TileCountY - 1) / 16 + 1, ProgramState.TileCountZ);
            Barriers[0].buffer = LightBuffers.LightCountTotal->vkHandle(); Barriers[0].size = LightBuffers.LightCountTotal->Size();
            Barriers[1].buffer = LightBuffers.LightCountOffsets->vkHandle(); Barriers[1].size = LightBuffers.LightCountOffsets->Size();
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 2, Barriers, 0, nullptr);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, Pipelines.ComputePipelines.Handles[NameIdxMap.at("LightList")]);
            vkCmdDispatch(cmd, (ProgramState.NumLights - 1) / 32 + 1, 1, 1);
        vkEndCommandBuffer(cmd);
        frame.ComputeCmd.firstFrame = false;
    }

    void ClusteredForward::submitComputePass(frame_data_t& frame) {
        VkSubmitInfo submission = vk_submit_info_base;
        submission.commandBufferCount = 1;
        submission.pCommandBuffers = &frame.ComputeCmd.Cmd;
        submission.waitSemaphoreCount = 1;
        submission.pWaitSemaphores = &acquiredBackBuffer.PreCompute;
        submission.signalSemaphoreCount = 1;
        submission.pSignalSemaphores = &acquiredBackBuffer.Compute;
        submission.pWaitDstStageMask = &ComputeFlags;
        vkQueueSubmit(device->ComputeQueue(), 1, &submission, frame.ComputeCmd.Fence);
    }

    void ClusteredForward::recordOnscreenPass(frame_data_t& frame) {
        VkResult result = vkWaitForFences(device->vkHandle(), 1, &frame.RenderCmd.Fence, VK_TRUE, static_cast<uint64_t>(1.0e9)); VkAssert(result);
        vkResetFences(device->vkHandle(), 1, &frame.RenderCmd.Fence);
        if (!frame.RenderCmd.firstFrame) {
            vkResetCommandBuffer(frame.RenderCmd.Cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        }

        offscreenViewport.maxDepth = 3000.0f;
        onscreenPass->UpdateBeginInfo(framebuffers[acquiredBackBuffer.idx]);
        auto& cmd = frame.RenderCmd.Cmd;
        constexpr VkCommandBufferBeginInfo begin_info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr };
        vkBeginCommandBuffer(cmd, &begin_info);
            vkCmdBeginRenderPass(cmd, &onscreenPass->BeginInfo(), VK_SUBPASS_CONTENTS_INLINE);
            vkCmdSetViewport(cmd, 0, 1, &offscreenViewport);
            vkCmdSetScissor(cmd, 0, 1, &offscreenScissor);
            const VkDescriptorSet first_two_sets[2]{ texelBufferSet->vkHandle(), frame.descriptor->vkHandle() };
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines.Opaque.Layout->vkHandle(), 0, 2, first_two_sets, 0, nullptr);
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines.Opaque.Pipeline->vkHandle());
            sponza->Render(DrawInfo{ cmd, Pipelines.Opaque.Layout->vkHandle(), true, true, 2 });
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipelines.Transparent.Pipeline->vkHandle());
            sponza->Render(DrawInfo{ cmd, Pipelines.Opaque.Layout->vkHandle(), false, true, 2 });
            vkCmdEndRenderPass(cmd);
            resetTexelBuffers(cmd);
        vkEndCommandBuffer(cmd);

        frame.RenderCmd.firstFrame = false;
    }

    void ClusteredForward::resetTexelBuffers(const VkCommandBuffer cmd) {
        std::vector<VkBufferMemoryBarrier> transfer_barriers{ 4, VkBufferMemoryBarrier{
            VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER, nullptr,
            VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
            VK_NULL_HANDLE, 0, 0
        } };

        transfer_barriers[0].buffer = LightBuffers.Flags->vkHandle(); transfer_barriers[0].size = LightBuffers.Flags->Size();
        transfer_barriers[1].buffer = LightBuffers.LightCounts->vkHandle(); transfer_barriers[1].size = LightBuffers.LightCounts->Size();
        transfer_barriers[2].buffer = LightBuffers.LightCountOffsets->vkHandle(); transfer_barriers[2].size = LightBuffers.LightCountOffsets->Size();
        transfer_barriers[3].buffer = LightBuffers.LightList->vkHandle(); transfer_barriers[3].size = LightBuffers.LightList->Size();

        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 4, transfer_barriers.data(), 0, nullptr);
        vkCmdFillBuffer(cmd, LightBuffers.Flags->vkHandle(), 0, LightBuffers.Flags->Size(), 0);
        vkCmdFillBuffer(cmd, LightBuffers.Bounds->vkHandle(), 0, LightBuffers.Bounds->Size(), 0);
        vkCmdFillBuffer(cmd, LightBuffers.LightCounts->vkHandle(), 0, LightBuffers.LightCounts->Size(), 0);
        vkCmdFillBuffer(cmd, LightBuffers.LightCountOffsets->vkHandle(), 0, LightBuffers.LightCountOffsets->Size(), 0);
        vkCmdFillBuffer(cmd, LightBuffers.LightCountTotal->vkHandle(), 0, LightBuffers.LightCountTotal->Size(), 0);
        vkCmdFillBuffer(cmd, LightBuffers.LightList->vkHandle(), 0, LightBuffers.LightList->Size(), 0);
        vkCmdFillBuffer(cmd, LightBuffers.LightCountsCompare->vkHandle(), 0, LightBuffers.LightCountsCompare->Size(), 0);
        for (auto& barrier : transfer_barriers) {
            // just invert the access masks before submitting another barrier.
            std::swap(barrier.srcAccessMask, barrier.dstAccessMask);
        }
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 4, transfer_barriers.data(), 0, nullptr);

    }

    void ClusteredForward::submitOnscreenPass(frame_data_t& frame) {
        VkSubmitInfo submission = vk_submit_info_base;
        submission.commandBufferCount = 1;
        submission.pCommandBuffers = &frame.RenderCmd.Cmd;
        submission.waitSemaphoreCount = 1;
        submission.pWaitSemaphores = &acquiredBackBuffer.Compute;
        submission.signalSemaphoreCount = 1;
        submission.pSignalSemaphores = &acquiredBackBuffer.Render;
        submission.pWaitDstStageMask = &RenderFlags;
        vkQueueSubmit(device->GraphicsQueue(), 1, &submission, frame.RenderCmd.Fence);
    }

    void ClusteredForward::recordCommands() {
        constexpr static VkDeviceSize Offsets[1]{ 0 };
        auto& frame_data = FrameData[CurrFrameDataIdx];

        recordPrecomputePass(frame_data);
        submitPrecomputePass(frame_data);
        recordComputePass(frame_data);
        submitComputePass(frame_data);
        recordOnscreenPass(frame_data);
        submitOnscreenPass(frame_data);

        CurrFrameDataIdx = (CurrFrameDataIdx + 1) % 3;
    }

    void ClusteredForward::presentBackBuffer() {
        VkPresentInfoKHR present_info = vk_present_info_base;
        present_info.pImageIndices = &acquiredBackBuffer.idx;
        present_info.pWaitSemaphores = &acquiredBackBuffer.Render;
        present_info.waitSemaphoreCount = 1;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain->vkHandle();
        VkResult result = vkQueuePresentKHR(device->GraphicsQueue(), &present_info);
        VkAssert(result);
    }

    void ClusteredForward::createFrameData() {
        const uint32_t num_frames = swapchain->ImageCount();
        for (uint32_t i = 0; i < num_frames; ++i) {
            FrameData.push_back(frame_data_t{ device.get(), i });
            FrameData.back().ComputeCmd.Cmd = computePool->GetCmdBuffer(i);
            FrameData.back().OffscreenCmd.Cmd = primaryPool->GetCmdBuffer(i * 2);
            FrameData.back().RenderCmd.Cmd = primaryPool->GetCmdBuffer(i * 2 + 1); 
        }  
    }

    void ClusteredForward::createBackbuffers() {
        for (size_t i = 0; i < swapchain->ImageCount(); ++i) {
            Backbuffers.push_back(std::move(backbuffer_data_t{ device.get() }));
        }
    }

    void ClusteredForward::createUBO() {
        ubo = std::make_unique<Buffer>(device.get());
        ubo->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sizeof(GlobalUBO));
    }

    void ClusteredForward::createLightBuffers() {
        constexpr static auto mem_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        uint32_t indices[2]{ 0, 0 };
        VkBufferCreateInfo buffer_info = vk_buffer_create_info_base;

        if (device->QueueFamilyIndices.Graphics != device->QueueFamilyIndices.Compute) {
            buffer_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
            indices[0] = device->QueueFamilyIndices.Graphics;
            indices[1] = device->QueueFamilyIndices.Compute;
            buffer_info.pQueueFamilyIndices = indices;
            buffer_info.queueFamilyIndexCount = 2;
        }
        else {
            buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            buffer_info.pQueueFamilyIndices = nullptr;
            buffer_info.queueFamilyIndexCount = 0;
        }

        buffer_info.usage = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        const uint32_t max_grid_count = ((ProgramState.ResolutionX - 1) / ProgramState.TileWidth + 1) * ((ProgramState.ResolutionY - 1) / ProgramState.TileHeight + 1) * ProgramState.TileCountZ;

        LightBuffers.Flags = std::make_unique<Buffer>(device.get());
        buffer_info.size = max_grid_count * sizeof(uint8_t);
        LightBuffers.Flags->CreateBuffer(buffer_info, mem_flags);
        LightBuffers.Flags->CreateView(VK_FORMAT_R8_UINT, LightBuffers.Flags->Size(), 0);

        LightBuffers.Bounds = std::make_unique<Buffer>(device.get());
        buffer_info.size = ProgramState.MaxLights * 6 * sizeof(uint32_t);
        LightBuffers.Bounds->CreateBuffer(buffer_info, mem_flags);
        LightBuffers.Bounds->CreateView(VK_FORMAT_R32_UINT, LightBuffers.Bounds->Size(), 0);

        LightBuffers.LightCounts = std::make_unique<Buffer>(device.get());
        buffer_info.size = max_grid_count * sizeof(uint32_t);
        LightBuffers.LightCounts->CreateBuffer(buffer_info, mem_flags);
        LightBuffers.LightCounts->CreateView(VK_FORMAT_R32_UINT, LightBuffers.LightCounts->Size(), 0);

        LightBuffers.LightCountTotal = std::make_unique<Buffer>(device.get());
        buffer_info.size = sizeof(uint32_t);
        LightBuffers.LightCountTotal->CreateBuffer(buffer_info, mem_flags);
        LightBuffers.LightCountTotal->CreateView(VK_FORMAT_R32_UINT, LightBuffers.LightCountTotal->Size(), 0);

        LightBuffers.LightCountOffsets = std::make_unique<Buffer>(device.get());
        buffer_info.size = max_grid_count * sizeof(uint32_t);
        LightBuffers.LightCountOffsets->CreateBuffer(buffer_info, mem_flags);
        LightBuffers.LightCountOffsets->CreateView(VK_FORMAT_R32_UINT, LightBuffers.LightCountOffsets->Size(), 0);

        LightBuffers.LightList = std::make_unique<Buffer>(device.get());
        buffer_info.size = 1024 * 1024 * sizeof(uint32_t);
        LightBuffers.LightList->CreateBuffer(buffer_info, mem_flags);
        LightBuffers.LightList->CreateView(VK_FORMAT_R32_UINT, LightBuffers.LightList->Size(), 0);

        LightBuffers.LightCountsCompare = std::make_unique<Buffer>(device.get());
        buffer_info.size = max_grid_count * sizeof(uint32_t);
        LightBuffers.LightCountsCompare->CreateBuffer(buffer_info, mem_flags);
        LightBuffers.LightCountsCompare->CreateView(VK_FORMAT_R32_UINT, LightBuffers.LightCountsCompare->Size(), 0);

    }

    GraphicsPipelineInfo PipelineInfo;
    static constexpr VkDynamicState States[2]{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

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

        info.RasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        info.RasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        info.DepthStencilInfo.depthTestEnable = VK_TRUE;
        info.DepthStencilInfo.depthWriteEnable = VK_TRUE;
        info.DepthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        info.DepthStencilInfo.stencilTestEnable = VK_FALSE;

        info.ColorBlendInfo.attachmentCount = 0;
        info.ColorBlendInfo.pAttachments = nullptr;
        info.ColorBlendInfo.logicOpEnable = VK_FALSE;
        info.ColorBlendInfo.logicOp = VK_LOGIC_OP_CLEAR;

        info.DynamicStateInfo.dynamicStateCount = 2;
        static constexpr VkDynamicState States[2]{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        info.DynamicStateInfo.pDynamicStates = States;

        auto pipeline_info = info.GetPipelineCreateInfo();
        pipeline_info.stageCount = 1;
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

        constexpr static VkDynamicState states[2]{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        PipelineInfo.DynamicStateInfo.dynamicStateCount = 2;
        PipelineInfo.DynamicStateInfo.pDynamicStates = states;

        VkGraphicsPipelineCreateInfo create_info = PipelineInfo.GetPipelineCreateInfo();
        create_info.layout = Pipelines.LightingOpaque.Layout->vkHandle();
        const VkPipelineShaderStageCreateInfo shader_infos[2]{ shaders.at("Light.vert")->PipelineInfo(), shaders.at("Light.frag")->PipelineInfo() };
        const uint16_t hash_id = static_cast<uint16_t>(std::hash<std::string>()("lighting-pipeline-opaque"));
        Pipelines.LightingOpaque.Cache = std::make_unique<PipelineCache>(device.get(), hash_id);
        create_info.subpass = 1;
        create_info.renderPass = computePass->vkHandle();
        create_info.stageCount = 2;
        create_info.pStages = shader_infos;

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

        PipelineInfo.RasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;

        PipelineInfo.DynamicStateInfo.dynamicStateCount = 2;
        constexpr static VkDynamicState states[2]{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        PipelineInfo.DynamicStateInfo.pDynamicStates = states;

        VkGraphicsPipelineCreateInfo create_info = PipelineInfo.GetPipelineCreateInfo();
        create_info.subpass = 0;
        create_info.layout = Pipelines.Opaque.Layout->vkHandle();
        create_info.renderPass = onscreenPass->vkHandle();

        const VkPipelineShaderStageCreateInfo stages[2]{ shaders.at("Clustered.vert")->PipelineInfo(), shaders.at("Clustered.frag")->PipelineInfo() };

        create_info.stageCount = 2;
        create_info.pStages = stages;

        Pipelines.Opaque.Pipeline = std::make_unique<GraphicsPipeline>(device.get());
        Pipelines.Opaque.Pipeline->Init(create_info, Pipelines.Opaque.Cache->vkHandle());

        PipelineInfo.RasterizationInfo.cullMode = VK_CULL_MODE_NONE;
        attachment_state.blendEnable = VK_FALSE;
        PipelineInfo.DepthStencilInfo.depthWriteEnable = VK_FALSE;

        Pipelines.Transparent.Pipeline = std::make_unique<GraphicsPipeline>(device.get());
        Pipelines.Transparent.Pipeline->Init(create_info, Pipelines.Opaque.Cache->vkHandle());

    }

    void ClusteredForward::createParticlePipeline() {

        GraphicsPipelineInfo PipelineInfo;
        PipelineInfo.AssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        
        constexpr static std::array<VkVertexInputAttributeDescription, 2> attributes{
            VkVertexInputAttributeDescription{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
            VkVertexInputAttributeDescription{ 1, 0, VK_FORMAT_R8G8B8A8_UNORM, sizeof(glm::vec4) }
        };

        constexpr static VkVertexInputBindingDescription binding {
            0, sizeof(glm::vec4) + 4 * sizeof(uint8_t), VK_VERTEX_INPUT_RATE_VERTEX
        };
    }

    void ClusteredForward::createComputePipelines() {
        Pipelines.ComputePipelines.Infos.fill(VkComputePipelineCreateInfo{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, nullptr, 0, VkPipelineShaderStageCreateInfo{}, VK_NULL_HANDLE, VK_NULL_HANDLE, -1 });
        Pipelines.ComputeLayout = std::make_unique<PipelineLayout>(device.get());
        const VkDescriptorSetLayout layouts[2]{ frameDataLayout->vkHandle(), texelBuffersLayout->vkHandle() };
        Pipelines.ComputeLayout->Create(layouts, 2);
        Pipelines.ComputePipelines.Infos[NameIdxMap.at("LightGrids")].layout = Pipelines.ComputeLayout->vkHandle();
        Pipelines.ComputePipelines.Infos[NameIdxMap.at("GridOffsets")].layout = Pipelines.ComputeLayout->vkHandle();
        Pipelines.ComputePipelines.Infos[NameIdxMap.at("LightList")].layout = Pipelines.ComputeLayout->vkHandle();
        Pipelines.ComputePipelines.Infos[NameIdxMap.at("LightGrids")].stage = shaders.at("LightGrid.comp")->PipelineInfo();
        Pipelines.ComputePipelines.Infos[NameIdxMap.at("GridOffsets")].stage = shaders.at("GridOffsets.comp")->PipelineInfo();
        Pipelines.ComputePipelines.Infos[NameIdxMap.at("LightList")].stage = shaders.at("LightList.comp")->PipelineInfo();
        Pipelines.ComputePipelines.Cache = std::make_unique<PipelineCache>(device.get(), static_cast<uint16_t>(std::hash<std::string>()("compute-pipeline-cache")));

        VkResult result = vkCreateComputePipelines(device->vkHandle(), Pipelines.ComputePipelines.Cache->vkHandle(),
            static_cast<uint32_t>(Pipelines.ComputePipelines.Infos.size()), Pipelines.ComputePipelines.Infos.data(),
            nullptr, Pipelines.ComputePipelines.Handles.data());
        VkAssert(result);
    }

    void ClusteredForward::createDescriptorPool() {
        descriptorPool = std::make_unique<DescriptorPool>(device.get(), swapchain->ImageCount() + 1);
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swapchain->ImageCount());
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, swapchain->ImageCount() * 2 + 7);
        descriptorPool->Create();
    }

    void ClusteredForward::createFrameDataSetLayout() {
        frameDataLayout = std::make_unique<DescriptorSetLayout>(device.get());
        constexpr static VkShaderStageFlags vfc_flags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
        constexpr static VkShaderStageFlags fc_flags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
        frameDataLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, vfc_flags, 0);
        frameDataLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, fc_flags, 1);
        frameDataLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, fc_flags, 2);

        for (auto& frame : FrameData) {
            frame.descriptor = std::make_unique<DescriptorSet>(device.get());
            frame.descriptor->AddDescriptorInfo(ubo->GetDescriptor(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);
            frame.descriptor->AddDescriptorInfo(frame.LightPositions->GetDescriptor(), frame.LightPositions->View(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1);
            frame.descriptor->AddDescriptorInfo(frame.LightColors->GetDescriptor(), frame.LightColors->View(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 2);
            frame.descriptor->Init(descriptorPool.get(), frameDataLayout.get());
        }
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
        texelBufferSet->AddDescriptorInfo(LightBuffers.Flags->GetDescriptor(), LightBuffers.Flags->View(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 0);
        texelBufferSet->AddDescriptorInfo(LightBuffers.Bounds->GetDescriptor(), LightBuffers.Bounds->View(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1);
        texelBufferSet->AddDescriptorInfo(LightBuffers.LightCounts->GetDescriptor(), LightBuffers.LightCounts->View(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 2);
        texelBufferSet->AddDescriptorInfo(LightBuffers.LightCountTotal->GetDescriptor(), LightBuffers.LightCountTotal->View(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 3);
        texelBufferSet->AddDescriptorInfo(LightBuffers.LightCountOffsets->GetDescriptor(), LightBuffers.LightCountOffsets->View(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 4);
        texelBufferSet->AddDescriptorInfo(LightBuffers.LightList->GetDescriptor(), LightBuffers.LightList->View(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 5);
        texelBufferSet->AddDescriptorInfo(LightBuffers.LightCountsCompare->GetDescriptor(), LightBuffers.LightCountsCompare->View(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 6);
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
        onscreenDescr[0].samples = VK_SAMPLE_COUNT_4_BIT;
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
        onscreenDescr[2].samples = VK_SAMPLE_COUNT_4_BIT;
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
        msaa = std::make_unique<Multisampling>(device.get(), swapchain.get(), VK_SAMPLE_COUNT_4_BIT);
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

}

int main(int argc, char* argv[]) {
    using namespace vpsk;
    using namespace vpr;
    ClusteredForward fwd("../rsrc/crytekSponza/sponza.obj");
    fwd.RenderLoop();
    return 0;
}