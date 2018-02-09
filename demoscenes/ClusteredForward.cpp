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
#include <memory>
#include <map>
#include <random>
namespace forward_plus {

    // https://mynameismjp.wordpress.com/2016/03/25/bindless-texturing-for-deferred-rendering-and-decals/

    using namespace vpr;
    using namespace vpsk;

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

    std::vector<Light> Lights;
    std::vector<glm::vec4> LightPositions;
    std::vector<glm::u8vec4> LightColors;

    class light_buffers_t {
    public:

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

    std::uniform_real_distribution<float> rand_distr;

    float random_unit_float() {
        return rand_distr(std::mt19937());
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
        const float light_vol = volume / static_cast<float>(ProgramState.NumLights);
        const float base_range = powf(light_vol, 1.0f / 3.0f);
        const float max_range = base_range * 3.0f;
        const float min_range = base_range / 1.5f;
        const glm::vec3 half_size = (model_bounds.Max() - model_bounds.Min()) * 0.50f;
        const float pos_radius = std::max(half_size.x, std::max(half_size.y, half_size.z));
        Lights.reserve(ProgramState.NumLights);
        for (uint32_t i = 0; i < ProgramState.NumLights; ++i) {
            glm::vec3 fcol = hue_to_rgb(random_range(0.0f,1.0f));
            fcol *= 1.30f;
            fcol -= 0.15f;
            const glm::u8vec4 color = float_to_uchar(fcol);
            const glm::vec3 position{ 
                random_range(-pos_radius, pos_radius), random_range(-pos_radius, pos_radius), random_range(-pos_radius, pos_radius) 
            };
            const float range = random_range(min_range, max_range);
            Lights.emplace_back(position, color, range);
        }
    }

    constexpr static std::array<const VkClearValue, 3> ClearValues {
        VkClearValue{ 0.0f, 0.0f, 0.0f, 1.0f },
        VkClearValue{ 0.0f, 0.0f, 0.0f, 1.0f },
        VkClearValue{ 1.0f, 0 }
    };

    struct clustered_forward_global_ubo_t {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
        glm::mat4 normal;
        glm::vec4 viewPosition;
        uint32_t NumLights;
    } ClusteredForwardGlobalUBO;

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

    enum class QueryType : uint8_t {
        DepthPass = 0,
        Clustering,
        CalcGrids,
        CalcOffsets,
        CalcList,
        Onscreen,
        Transfer
    };

    struct query_data_t {
        std::array<uint32_t, 2> DepthPass;
        std::array<uint32_t, 2> Clustering;
        std::array<uint32_t, 2> CalcLightGrids;
        std::array<uint32_t, 2> CalcGridOffsets;
        std::array<uint32_t, 2> CalcLightList;
        std::array<uint32_t, 2> Onscreen;
        std::array<uint32_t, 2> Transfer;
    };

    struct frame_data_t {

        frame_data_t(const Device* dvc, uint32_t idx);

        struct cmd_buffer_block_t {
            VkCommandBuffer Cmd = VK_NULL_HANDLE;
            VkFence Fence = VK_NULL_HANDLE;
        };

        cmd_buffer_block_t OffscreenCmd;
        cmd_buffer_block_t ComputeCmd;
        cmd_buffer_block_t RenderCmd;
        VkQueryPool QueryPool;
        query_data_t QueryResults;
        const Device* device;
        std::unique_ptr<Buffer> LightPositions;
        std::unique_ptr<Buffer> LightColors;
        std::unique_ptr<Buffer> UBO;
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
        std::unique_ptr<PipelineLayout> PipelineLayout;
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
    };

    class ClusteredForward : public BaseScene {
    public:

        void Render();

    private:

        void createShaders();
        void createDepthPipeline();
        void createLightingOpaquePipeline();
        void createLightingTransparentPipeline();
        void createOpaquePipeline();
        void createTransparentPipeline();
        void createParticlePipeline();
        void createComputePipelines();

        void createComputeCmdPool();
        void createPrimaryCmdPool();
        void createSecondaryCmdPool();

        void createDescriptorPool();
        void createFrameDataSetLayout();
        void createFrameDataDescriptorSet();
        void createTexelBufferSetLayout();
        void createTexelBufferDescriptorSet();

        void createFrameData();

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

        light_buffers_t LightBuffers;
        clustered_forward_pipelines_t Pipelines;
        std::unique_ptr<DescriptorPool> descriptorPool;
        std::unique_ptr<DescriptorSetLayout> frameDataLayout;
        std::unique_ptr<DescriptorSetLayout> texelBuffersLayout;
        std::unique_ptr<CommandPool> primaryPool;
        std::unique_ptr<CommandPool> computePool;
        std::unique_ptr<Image> offscreenRenderTarget;
        std::unique_ptr<Framebuffer> offscreenFramebuffer;

        std::array<std::unique_ptr<DescriptorSet>, 3> frameSets;
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

        std::vector<frame_data_t> FrameData;
        VkSubmitInfo OffscreenSubmit, ComputeSubmit, RenderSubmit;
        std::map<std::string, std::unique_ptr<ShaderModule>> shaders;

        constexpr static VkPipelineStageFlags OffscreenFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        constexpr static VkPipelineStageFlags ComputeFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
        constexpr static VkPipelineStageFlags RenderFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        
    };
    
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

    void ClusteredForward::createFrameData() {
        const uint32_t num_frames = swapchain->ImageCount;
        for (uint32_t i = 0; i < num_frames; ++i) {
            FrameData.push_back(frame_data_t{ device.get(), i });
        }
        
    }

    frame_data_t::frame_data_t(const Device* dvc, const uint32_t _idx)  : idx(_idx), LightPositions(std::make_unique<Buffer>(dvc)), 
        LightColors(std::make_unique<Buffer>(dvc)), UBO(std::make_unique<Buffer>(dvc)), device(dvc) {
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
        LightPositions->CreateView(VK_FORMAT_R32G32B32A32_SFLOAT, LightPositions->Size(), 0);
        create_info.size = sizeof(uint8_t) * 4 * ProgramState.MaxLights;
        LightColors->CreateBuffer(create_info, mem_flags);
        LightColors->CreateView(VK_FORMAT_R8G8B8A8_UNORM, LightColors->Size(), 0);

        create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        create_info.size = sizeof(clustered_forward_global_ubo_t);
        UBO->CreateBuffer(create_info, mem_flags);

        VkFenceCreateInfo fence_info = vk_fence_create_info_base;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VkResult result = vkCreateFence(device->vkHandle(), &fence_info, nullptr, &OffscreenCmd.Fence); VkAssert(result);
        result = vkCreateFence(device->vkHandle(), &fence_info, nullptr, &ComputeCmd.Fence); VkAssert(result);
        result = vkCreateFence(device->vkHandle(), &fence_info, nullptr, &RenderCmd.Fence); VkAssert(result);
    }

    void ClusteredForward::createDepthPipeline() {

        Pipelines.Depth.PipelineLayout = std::make_unique<PipelineLayout>(device.get());
        Pipelines.Depth.PipelineLayout->Create({ frameDataLayout->vkHandle() });

        GraphicsPipelineInfo info;
        info.VertexInfo.vertexAttributeDescriptionCount = 1;
        info.VertexInfo.vertexBindingDescriptionCount = 1;
        // just use position input.
        static const VkVertexInputAttributeDescription attr = vertex_t::attributeDescriptions[0];
        static const VkVertexInputBindingDescription bind = vertex_t::bindingDescriptions[0];
        info.VertexInfo.pVertexAttributeDescriptions = &attr;
        info.VertexInfo.pVertexBindingDescriptions = &bind;

        auto pipeline_info = info.GetPipelineCreateInfo();
        pipeline_info.stageCount = 1;
        pipeline_info.pStages = &shaders.at("Simple.vert")->PipelineInfo();
        pipeline_info.layout = Pipelines.Depth.PipelineLayout->vkHandle();

        Pipelines.Depth.Pipeline = std::make_unique<GraphicsPipeline>(device.get(), pipeline_info, Pipelines.Depth.Cache->vkHandle());
    }

    void ClusteredForward::createComputePipelines() {
        Pipelines.ComputePipelines.Infos.fill(VkComputePipelineCreateInfo{ 
            VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, nullptr, 0, VkPipelineShaderStageCreateInfo{}, VK_NULL_HANDLE, VK_NULL_HANDLE, -1
        });
        Pipelines.ComputePipelines.Infos[NameIdxMap.at("LightGrids")].layout = Pipelines.ComputePipelines.PipelineLayout->vkHandle();
        Pipelines.ComputePipelines.Infos[NameIdxMap.at("GridOffsets")].layout = Pipelines.ComputePipelines.PipelineLayout->vkHandle();
        Pipelines.ComputePipelines.Infos[NameIdxMap.at("LightList")].layout = Pipelines.ComputePipelines.PipelineLayout->vkHandle();
        Pipelines.ComputePipelines.Infos[NameIdxMap.at("LightGrids")].stage = shaders.at("LightGrids.comp")->PipelineInfo();
        Pipelines.ComputePipelines.Infos[NameIdxMap.at("GridOffsets")].stage = shaders.at("GridOffsets.comp")->PipelineInfo();
        Pipelines.ComputePipelines.Infos[NameIdxMap.at("LightList")].stage = shaders.at("LightList.comp")->PipelineInfo();
        Pipelines.ComputePipelines.Cache = std::make_unique<PipelineCache>(device.get(), 
            static_cast<uint16_t>(std::hash<std::string>()("compute-pipeline-cache")));
        
        VkResult result = vkCreateComputePipelines(device->vkHandle(), Pipelines.ComputePipelines.Cache->vkHandle(), 
            static_cast<uint32_t>(Pipelines.ComputePipelines.Infos.size()), Pipelines.ComputePipelines.Infos.data(), 
            nullptr, Pipelines.ComputePipelines.Handles.data());
        VkAssert(result);
    }

    void ClusteredForward::createLightingOpaquePipeline() {
        Pipelines.LightingOpaque.PipelineLayout = std::make_unique<PipelineLayout>(device.get());
        Pipelines.LightingOpaque.PipelineLayout->Create({ frameDataLayout->vkHandle(), texelBuffersLayout->vkHandle() });
    }

    void ClusteredForward::createDescriptorPool() {
        descriptorPool = std::make_unique<DescriptorPool>(device.get(), swapchain->ImageCount() + 1);
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swapchain->ImageCount());
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, swapchain->ImageCount * 2 + 7);
        descriptorPool->Create();
    }

    void ClusteredForward::createFrameDataSetLayout() {
        frameDataLayout = std::make_unique<DescriptorSetLayout>(device.get());
        constexpr static VkShaderStageFlags vfc_flags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
        constexpr static VkShaderStageFlags fc_flags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
        frameDataLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, vfc_flags, 0);
        frameDataLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, fc_flags, 1);
        frameDataLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, fc_flags, 2);
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
        texelBufferSet->AddDescriptorInfo(LightBuffers.Flags->GetDescriptor(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 0);
        texelBufferSet->AddDescriptorInfo(LightBuffers.Bounds->GetDescriptor(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1);
        texelBufferSet->AddDescriptorInfo(LightBuffers.LightCounts->GetDescriptor(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 2);
        texelBufferSet->AddDescriptorInfo(LightBuffers.LightCountTotal->GetDescriptor(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 3);
        texelBufferSet->AddDescriptorInfo(LightBuffers.LightCountOffsets->GetDescriptor(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 4);
        texelBufferSet->AddDescriptorInfo(LightBuffers.LightList->GetDescriptor(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 5);
        texelBufferSet->AddDescriptorInfo(LightBuffers.LightCountsCompare->GetDescriptor(), VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 6);
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
        onscreenDescr[0].format = swapchain->ColorFormat();
        onscreenDescr[0].samples = VK_SAMPLE_COUNT_4_BIT;
        onscreenDescr[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        onscreenDescr[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        onscreenDescr[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        onscreenDescr[1].format = swapchain->ColorFormat();
        onscreenDescr[1].samples = VK_SAMPLE_COUNT_1_BIT;
        onscreenDescr[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        onscreenDescr[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        onscreenDescr[2].format = device->FindDepthFormat();
        onscreenDescr[2].samples = VK_SAMPLE_COUNT_4_BIT;
        onscreenDescr[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        onscreenDescr[2].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    void ClusteredForward::createOnscreenAttachmentReferences() {
        onscreenReferences[0] = VkAttachmentReference{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        onscreenReferences[1] = VkAttachmentReference{ 2, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
        onscreenReferences[2] = VkAttachmentReference{ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    }

    void ClusteredForward::createOnscreenSubpassDescription() {
        onscreenSbDescr = vk_subpass_description_base;
        onscreenSbDescr.colorAttachmentCount = 1;
        onscreenSbDescr.pColorAttachments = &onscreenReferences[0];
        onscreenSbDescr.pResolveAttachments = &onscreenReferences[2];
        onscreenSbDescr.pDepthStencilAttachment = &onscreenReferences[1];
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

        computeSbDescr[1] = computeSbDescr[1];
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
        computePass->SetupBeginInfo(ClearValues.data(), ClearValues.size(), swapchain->Extent());
    }

    void ClusteredForward::createShaders() {
        const std::string prefix("clustered_forward/");
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
        create_info.imageType = VK_IMAGE_TYPE_2D;
        create_info.format = device->FindDepthFormat();
        create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        offscreenRenderTarget->Create(create_info);
        offscreenRenderTarget->CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    void ClusteredForward::createOffscreenFramebuffer() {
        VkFramebufferCreateInfo create_info = vk_framebuffer_create_info_base;
        create_info.attachmentCount = 1;
        create_info.pAttachments = &offscreenRenderTarget->View();
        create_info.renderPass = computePass->vkHandle();
        offscreenFramebuffer = std::make_unique<Framebuffer>(device.get(), create_info);
    }

}
