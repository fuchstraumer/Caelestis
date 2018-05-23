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
#include "scene/Window.hpp"
#include "resources/Multisampling.hpp"
#include "render/DepthStencil.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "resources/LightBuffers.hpp"
#include "render/FrameData.hpp"
#include "render/BackBuffer.hpp"
#include "camera/Camera.hpp"
#include "core/ShaderGroup.hpp"
#include "core/ShaderPack.hpp"
#include "core/ShaderResource.hpp"
#include <memory>
#include <set>
#include <map>
#include <random>
#include <deque>
#include <array>
#include <experimental/filesystem>

#include "util/easylogging++.h"
INITIALIZE_EASYLOGGINGPP
namespace volumetric_forward {

    constexpr uint32_t LightGridBlockSize = 32;
    constexpr uint32_t ClusterGridBlockSize = 64;

    struct alignas(4) light_counts_t {
        uint32_t NumPointLights{ 0 };
        uint32_t NumSpotLights{ 0 };
        uint32_t NumDirectionalLights{ 0 };
        uint32_t GetMaxLights() const noexcept {
            return std::max(NumPointLights, std::max(NumSpotLights, NumDirectionalLights));
        }
    } LightCounts;

    struct alignas(4) dispatch_params_t {
        glm::uvec3 NumThreadGroups{ 0, 0, 0 };
        const uint32_t Padding0{ 0 };
        glm::uvec3 NumThreads{ 0, 0, 0 };
        const uint32_t Padding1{ 0 };
    } DispatchParams;

    struct alignas(16) frustum_t {
        std::array<glm::vec4, 4> Planes;
    } Frustum;

    struct alignas(4) cluster_data_t {
        glm::uvec3 GridDim{ 0, 0, 0 };
        float ViewNear{ 0.0f };
        glm::uvec2 Size{ 0, 0 };
        float NearK{ 0.0f };
        float LogGridDimY{ 0.0f };
    };

    struct alignas(4) SortParams {
        uint32_t NumElements{ 0 };
        uint32_t ChunkSize{ 0 };
    };

    struct alignas(4) BVHParams {
        uint32_t PointLightLevels{ 0 };
        uint32_t SpotLightLevels{ 0 };
        uint32_t ChildLevel{ 0 };
    };

    struct render_target_t {
        render_target_t() = default;
        ~render_target_t() = default;
        void CreateRT(const vpr::Device* dvc, const vpr::Swapchain* swap, const VkRenderPass& pass, const VkImageUsageFlagBits usage, 
            const VkFormat fmt);

        std::unique_ptr<vpr::Image> Image{ nullptr };
        std::unique_ptr<vpr::Framebuffer> Framebuffer{ nullptr };
    } DepthOnlyRenderTarget;

    struct compute_pipeline_t {
        compute_pipeline_t() = default;
        ~compute_pipeline_t() = default;

        void BindSets(const VkCommandBuffer& cmd);    

        VkPipeline Handle{ VK_NULL_HANDLE };
        VkComputePipelineCreateInfo Info{ vpr::vk_compute_pipeline_create_info_base };
        std::unique_ptr<vpr::PipelineLayout> PipelineLayout{ nullptr };
        std::unique_ptr<vpr::DescriptorSetLayout> SetLayout{ nullptr };
        std::unique_ptr<vpr::PipelineCache> Cache{ nullptr };
        std::unordered_map<std::string, vpr::Buffer*> UsedBuffers;
        std::unordered_map<std::string, vpr::DescriptorSet*> Sets;
    };

    struct graphics_pipeline_t {
        graphics_pipeline_t() = default;
        ~graphics_pipeline_t() = default;

    };

    struct compute_pipelines_t {
        std::unordered_map<std::string, compute_pipeline_t> Handles;
        compute_pipeline_t& at(const std::string& key) {
            return Handles.at(key);
        }
    } ComputePipelines;

    std::unordered_map<std::string, std::unique_ptr<vpr::DescriptorSet>> DescriptorSets;
    std::unordered_map<std::string, std::unique_ptr<vpr::Buffer>> UniformBuffers;
    std::unordered_map<std::string, std::unique_ptr<vpr::Buffer>> StorageTexelBuffers;
    std::unordered_map<std::string, std::unique_ptr<vpr::Buffer>> StorageBuffers;

    constexpr static uint32_t SORTING_NUM_THREADS_PER_THREAD_GROUP = 256;
    constexpr static uint32_t SORT_NUM_VALUES_PER_THREAD = 8;

    constexpr static std::array<uint32_t, 6> NumLevelNodes{ 1, 32, 1024, 32768, 1048576, 33554432 };
    constexpr static std::array<uint32_t, 6> NumBVHNodes{ 1, 33, 1057, 33825, 1082401, 34636833 };

    uint32_t GetNumLevels(const uint32_t num_leaves) {
        static const double log32f = std::log(32.0);

        if (num_leaves > 0) {
            return static_cast<uint32_t>(std::ceil(std::log(static_cast<double>(num_leaves)) / log32f));
        }
        else {
            return 0;
        }
    }

    uint32_t GetNumNodes(const uint32_t num_leaves) {
        const uint32_t num_levels = GetNumLevels(num_leaves);
        if (num_levels > 0 && num_levels < NumBVHNodes.size()) {
            return NumBVHNodes[num_levels - 1];
        }
        else {
            return 0;
        }
    }

    void MergeSort(const VkCommandBuffer& cmd, vpr::Buffer* src_keys, vpr::Buffer* src_values, vpr::Buffer* dst_keys, 
        vpr::Buffer* dst_values, const uint32_t num_values, const uint32_t chunk_size) {
        
        auto& merge_path_partitions = StorageTexelBuffers.at("MergePathPartitions");
        auto& merge_partitions_pipeline = ComputePipelines.at("MergePathPartitions");
        auto& merge_sort_pipeline = ComputePipelines.at("MergeSort");

        constexpr uint32_t numThreadsPerGroup = SORTING_NUM_THREADS_PER_THREAD_GROUP;
        constexpr uint32_t numValuesPerThread = SORT_NUM_VALUES_PER_THREAD;
        constexpr uint32_t numValuesPerThreadGroup = numThreadsPerGroup * numValuesPerThread;

        uint32_t num_chunks = static_cast<uint32_t>(std::ceil(static_cast<double>(num_values) / static_cast<double>(chunk_size)));
        uint32_t pass = 0;
        uint32_t chunk_size_local = chunk_size;

        while (num_chunks > 1) {

            const SortParams params{ num_values, chunk_size_local };
            const uint32_t num_sort_groups = num_chunks / 2;
            uint32_t num_thread_groups_per_sort_group = static_cast<uint32_t>(
                std::ceil(static_cast<double>(chunk_size * 2) / static_cast<double>(numValuesPerThreadGroup))
            );

            {
                merge_path_partitions->Fill(cmd, merge_path_partitions->Size(), 0, 0);
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, merge_partitions_pipeline.Handle);
                merge_partitions_pipeline.BindSets(cmd);
                uint32_t num_merge_path_partitions_per_sort_group = num_thread_groups_per_sort_group + 1u;
                uint32_t total_merge_path_partitions = num_merge_path_partitions_per_sort_group * num_sort_groups;
                uint32_t num_thread_groups = static_cast<uint32_t>(std::ceil(static_cast<double>(total_merge_path_partitions) / static_cast<double>(numThreadsPerGroup)));
                vkCmdDispatch(cmd, num_thread_groups, 1, 1);
                const VkBufferMemoryBarrier barrier{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER, nullptr, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_QUEUE_FAMILY_IGNORED,
                    VK_QUEUE_FAMILY_IGNORED, merge_path_partitions->vkHandle(), 0, merge_path_partitions->Size() };
                vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 1, &barrier, 0, nullptr);

            }

            {
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, merge_sort_pipeline.Handle);
                uint32_t numValuesPerSortGroup = std::min(chunk_size_local * 2, num_values);
                num_thread_groups_per_sort_group = static_cast<uint32_t>(std::ceil(static_cast<double>(numValuesPerSortGroup) / static_cast<double>(numValuesPerThreadGroup)));
                vkCmdDispatch(cmd, num_thread_groups_per_sort_group * num_sort_groups, 1, 1);

            }

            chunk_size_local *= 2;
            num_chunks = static_cast<uint32_t>(std::ceil(static_cast<double>(num_values) / static_cast<double>(chunk_size_local)));

        }

        // TODO: How to easily swap bindings so that src becomes dst and dst becomes src?
    }

    void UpdateLights(const VkCommandBuffer& cmd) {
        auto& update_pipeline = ComputePipelines.at("UpdateLights");       
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, update_pipeline.Handle);
        update_pipeline.BindSets(cmd);
        uint32_t num_groups_x = std::max(LightCounts.NumPointLights, std::max(LightCounts.NumDirectionalLights, LightCounts.NumSpotLights));
        num_groups_x = static_cast<uint32_t>(std::ceil(static_cast<double>(num_groups_x) / 1024.0));
        vkCmdDispatch(cmd, num_groups_x, 1, 1);
    }

    void ReduceLights(const VkCommandBuffer& cmd) {
        // Descriptor sets from previous binding call are all we should still need.
        auto& reduce_pipeline = ComputePipelines.at("ReduceLights0");
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, reduce_pipeline.Handle);
        const uint32_t num_thread_groups = static_cast<uint32_t>(std::min(static_cast<int>(std::ceil(static_cast<double>(LightCounts.GetMaxLights()) / 512.0)), 512));
        dispatch_params_t params0;
        params0.NumThreadGroups = glm::uvec3{ num_thread_groups, 1, 1 };
        params0.NumThreads = glm::uvec3{ 512 * num_thread_groups, 1, 1 }; 
        UniformBuffers.at("DispatchParams")->Update(cmd, sizeof(dispatch_params_t), 0, &params0);
        vkCmdDispatch(cmd, num_thread_groups, 1, 1);
        dispatch_params_t params1;
        params1.NumThreadGroups = glm::uvec3(1, 1, 1);
        params1.NumThreads = glm::uvec3(512, 1, 1);
        UniformBuffers.at("DispatchParams")->Update(cmd, sizeof(dispatch_params_t), 0, &params1);
        vkCmdDispatch(cmd, 1, 1, 1);    
    }

    void ComputeMortonCodes(const VkCommandBuffer& cmd) {
        auto& morton_pipeline = ComputePipelines.at("ComputeMortonCodes");
        // Only need to bind this single extra set
        morton_pipeline.BindSets(cmd);
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, morton_pipeline.Handle);
        const uint32_t num_thread_groups = static_cast<uint32_t>(std::ceil(static_cast<double>(LightCounts.GetMaxLights()) / 1024.0));
        vkCmdDispatch(cmd, num_thread_groups, 1, 1);
    }

    void SortByMortonCodes(const VkCommandBuffer& cmd) {
        auto& radix_pipeline = ComputePipelines.at("RadixSort");

    }

    void BuildLightBVH(const VkCommandBuffer& cmd) {

    }

    void ExecutePrecomputeShaders(const VkCommandBuffer& cmd) {
        UpdateLights(cmd);
        ReduceLights(cmd);
        ComputeMortonCodes(cmd);
        SortByMortonCodes(cmd);
        BuildLightBVH(cmd);
    }

    void ComputeGridFrustums(const VkCommandBuffer& cmd, const uint32_t _screen_width, const uint32_t _screen_height) {
        const uint32_t screen_width = std::max(_screen_width, 1u);
        const uint32_t screen_height = std::max(_screen_height, 1u);

        const glm::uvec3 num_threads = glm::uvec3(
            static_cast<uint32_t>(std::ceil(static_cast<double>(screen_width) / 1024.0)), 
            static_cast<uint32_t>(std::ceil(static_cast<double>(screen_height) / 1024.0)), 
            1u
        );

        const glm::uvec3 num_thread_groups = glm::uvec3(
            static_cast<uint32_t>(std::ceil(static_cast<double>(num_threads.x) / static_cast<double>(LightGridBlockSize))),
            static_cast<uint32_t>(std::ceil(static_cast<double>(num_threads.y) / static_cast<double>(LightGridBlockSize))),
            1u
        );

    }

    void UpdateClusterGrid(const VkCommandBuffer& cmd, const vpsk::Camera& cam, const uint32_t screen_width, const uint32_t screen_height) {
        const uint32_t cluster_dim_x = static_cast<uint32_t>(std::ceil(static_cast<double>(screen_width) / static_cast<double>(ClusterGridBlockSize)));
        const uint32_t cluster_dim_y = static_cast<uint32_t>(std::ceil(static_cast<double>(screen_height) / static_cast<double>(ClusterGridBlockSize)));

        const float s_d = 2.0f * std::tan(cam.GetFOV() * 0.50f) / static_cast<float>(cluster_dim_y);
        const float log_dim_y = 1.0f / glm::log(1.0f + s_d);

        const float log_depth = glm::log(cam.GetNearPlane() / cam.GetFarPlane());
        const uint32_t cluster_dim_z = static_cast<uint32_t>(glm::floor(log_depth * log_dim_y));

        cluster_data_t cluster_data{
            glm::uvec3{ cluster_dim_x, cluster_dim_y, cluster_dim_z },
            cam.GetNearPlane(),
            glm::uvec2{ ClusterGridBlockSize, ClusterGridBlockSize },
            1.0f + s_d,
            log_dim_y
        };

        const uint32_t num_clusters = cluster_dim_x * cluster_dim_y * cluster_dim_z;
        StorageTexelBuffers.at("ClusterFlags")->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(uint8_t) * num_clusters);
        StorageTexelBuffers.at("ClusterFlags")->CreateView(VK_FORMAT_R8_UINT, StorageTexelBuffers.at("ClusterFlags")->Size(), 0);
        StorageTexelBuffers.at("UniqueClusters")->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(uint32_t) * num_clusters);
        StorageTexelBuffers.at("UniqueClusters")->CreateView(VK_FORMAT_R32_UINT, StorageTexelBuffers.at("UniqueClusters")->Size(), 0);
        StorageTexelBuffers.at("PreviousUniqueClusters")->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(uint32_t) * num_clusters);
        StorageTexelBuffers.at("PreviousUniqueClusters")->CreateView(VK_FORMAT_R32_UINT, StorageTexelBuffers.at("PreviousUniqueClusters")->Size(), 0);

        // TODO: Generate lights, update clusters now. Need some kind of byte address buffer?
    }

    void ClusteredPrePass(const VkCommandBuffer& cmd) {

    }

    void compute_pipeline_t::BindSets(const VkCommandBuffer & cmd) {
        std::vector<VkDescriptorSet> handles;
        for (auto& set : this->Sets) {
            handles.emplace_back(set.second->vkHandle());
        }
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, PipelineLayout->vkHandle(), 0, static_cast<uint32_t>(handles.size()), handles.data(), 0, nullptr);
    }
}

static int screen_x() {
    return 1440;
}

static int screen_y() {
    return 720;
}


static vpsk::Camera camera;

static double fov() {
    return camera.GetFOV();
}

static double near_plane() {
    return camera.GetNearPlane();
}

static double far_plane() {
    return camera.GetFarPlane();
}

constexpr VkApplicationInfo app_info{
    VK_STRUCTURE_TYPE_APPLICATION_INFO,
    nullptr,
    "VolumetricForward",
    VK_MAKE_VERSION(0,1,0),
    "VulpesSceneKit",
    VK_MAKE_VERSION(0,1,0),
    VK_API_VERSION_1_1
};

int main(int argc, char* argv[]) {
    using namespace volumetric_forward;
    using namespace st;
    using namespace vpsk;
    using namespace vpr;
    ImGui::CreateContext();
    auto window = std::make_unique<vpsk::Window>(1280, 720, "RendergraphSketch");
    auto instance = std::make_unique<vpr::Instance>(false, &app_info, window->glfwWindow());
    auto device = std::make_unique<vpr::Device>(instance.get(), instance->GetPhysicalDevice(), true);

    auto& callbacks = ShaderPack::RetrievalCallbacks();
    callbacks.GetScreenSizeX = &screen_x;
    callbacks.GetScreenSizeY = &screen_y;
    callbacks.GetFOVY = &fov;
    callbacks.GetZNear = &near_plane;
    callbacks.GetZFar = &far_plane;
    ShaderPack pack("../third_party/shadertools/fragments/volumetric_forward/ShaderPack.lua");
    std::vector<std::string> group_names;
    {
        auto names = pack.GetShaderGroupNames();
        for (size_t i = 0; i < names.NumStrings; ++i) {
            group_names.emplace_back(names.Strings[i]);
        }
    }

    std::vector<std::string> resource_block_names;
    {
        auto retrieved_names = pack.GetResourceGroupNames();
        for (size_t i = 0; i < retrieved_names.NumStrings; ++i) {
            resource_block_names.emplace_back(retrieved_names.Strings[i]);
        }
    }

    std::unordered_map<std::string, std::vector<const ShaderResource*>> resources;
    {
        for (const auto& block_name : resource_block_names) {
            size_t num_resources = 0;
            pack.GetResourceGroupPointers(block_name.c_str(), &num_resources, nullptr);
            std::vector<const ShaderResource*> vec(num_resources);
            pack.GetResourceGroupPointers(block_name.c_str(), &num_resources, vec.data());
            resources.emplace(block_name, vec);
        }
    }

    std::unique_ptr<DescriptorPool> pool = std::make_unique<DescriptorPool>(device.get(), resources.size());
    const auto& rsrc_counts = pack.GetTotalDescriptorTypeCounts();
    pool->AddResourceType(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, rsrc_counts.UniformBuffers);
    pool->AddResourceType(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, rsrc_counts.StorageTexelBuffers);
    pool->AddResourceType(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, rsrc_counts.StorageBuffers);
    pool->AddResourceType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, rsrc_counts.CombinedImageSamplers);
    pool->Create();

    for (const auto& block_name : resource_block_names) {
        for (const auto& resource : resources.at(block_name)) {
            const std::string rsrc_name = resource->GetName();
            if (resource->GetType() == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                UniformBuffers.emplace(rsrc_name, std::make_unique<Buffer>(device.get()));
                UniformBuffers.at(rsrc_name)->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VkDeviceSize(resource->GetAmountOfMemoryRequired()));
            }
            else if (resource->GetType() == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) {
                StorageTexelBuffers.emplace(rsrc_name, std::make_unique<Buffer>(device.get()));
                StorageTexelBuffers.at(rsrc_name)->CreateBuffer(VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VkDeviceSize(resource->GetAmountOfMemoryRequired()));
                StorageTexelBuffers.at(rsrc_name)->CreateView(resource->GetFormat(), resource->GetAmountOfMemoryRequired(), 0);
            }
            else if (resource->GetType() == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
                StorageBuffers.emplace(rsrc_name, std::make_unique<Buffer>(device.get()));
                StorageBuffers.at(rsrc_name)->CreateBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VkDeviceSize(resource->GetAmountOfMemoryRequired()));
            }
        }
    }


    return 0;
}