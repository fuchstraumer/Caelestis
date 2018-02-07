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
#include <memory>

namespace forward_plus {

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

    class LightBuffers {
    public:

        LightBuffers(const Device* dvc);
        void CreateBuffers();
        std::unique_ptr<Buffer> Flags;
        std::unique_ptr<Buffer> Bounds;
        std::unique_ptr<Buffer> LightCounts;
        std::unique_ptr<Buffer> LightCountTotal;
        std::unique_ptr<Buffer> LightCountOffsets;
        std::unique_ptr<Buffer> LightList;
        std::unique_ptr<Buffer> LightCountsCompare;
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

        frame_data_t(const Device* dvc);

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
    };

    class ClusteredForward : public BaseScene {
    public:

    private:
        std::unique_ptr<CommandPool> computePool;
        std::vector<frame_data_t> FrameData;
        VkPipelineStageFlags OffscreenFlags, ComputeFlags, RenderFlags;
        VkSubmitInfo OffscreenSubmit, ComputeSubmit, RenderSubmit;

        void createFrameData();
    };
    
    LightBuffers::LightBuffers(const Device* dvc) : Flags(std::make_unique<Buffer>(dvc)), Bounds(std::make_unique<Buffer>(dvc)), 
        LightCounts(std::make_unique<Buffer>(dvc)), LightCountTotal(std::make_unique<Buffer>(dvc)), LightCountOffsets(std::make_unique<Buffer>(dvc)),
        LightList(std::make_unique<Buffer>(dvc)), LightCountsCompare(std::make_unique<Buffer>(dvc)) {}

    void LightBuffers::CreateBuffers() {
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
        FrameData.resize(num_frames, device.get());
        for (auto& data : FrameData) {
            data.Create();
        }
    }

    frame_data_t::frame_data_t(const Device* dvc)  : LightPositions(std::make_unique<Buffer>(dvc)), LightColors(std::make_unique<Buffer>(dvc)),
        UBO(std::make_unique<Buffer>(dvc)), device(dvc) {
        const VkMemoryPropertyFlags mem_flags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
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
    }

}