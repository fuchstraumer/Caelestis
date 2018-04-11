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

    class DescriptorSetDetail {

        DescriptorSetDetail(const vpr::Device* parent);

        void AddResource(std::string name, VkDescriptorBufferInfo info, const VkDescriptorType type, const uint32_t binding_idx);
        void AddResource(std::string name, VkDescriptorBufferInfo info, const VkBufferView view, const VkDescriptorType type, const uint32_t binding_idx);
        void AddResource(std::string name, VkDescriptorImageInfo info, const VkDescriptorType type, const uint32_t binding_idx);

        void Init(const vpr::DescriptorPool* parent_pool, const vpr::DescriptorSetLayout* set_layout);
        const VkDescriptorSet& vkHandle() const noexcept;

    private:
    
        std::unique_ptr<vpr::DescriptorSet> descriptor;
        std::set<std::string> resources;
    };

    struct resources_collection_t {
        std::unordered_map<std::string, std::unique_ptr<vpr::Buffer>> Resources;
        std::unique_ptr<vpr::DescriptorSet> Descriptor;
        std::unique_ptr<vpr::Buffer>& at(const std::string& key) {
            return Resources.at(key);
        }
    };

    resources_collection_t ClusterResources{
        std::unordered_map<std::string, std::unique_ptr<vpr::Buffer>> {
            { "ClusterData", nullptr },
            { "ClusterFlags", nullptr },
            { "PointLightIndexList", nullptr },
            { "SpotLightIndexList", nullptr },
            { "PointLightGrid", nullptr },
            { "SpotLightGrid", nullptr },
            { "UniqueClustersCounter", nullptr },
            { "UniqueClusters", nullptr },
            { "PreviousUniqueClusters", nullptr }
        },
        std::unique_ptr<vpr::DescriptorSet>{ nullptr }
    };

    resources_collection_t LightResources{
        std::unordered_map<std::string, std::unique_ptr<vpr::Buffer>> {
            { "LightCounts", nullptr },
            { "PointLights", nullptr },
            { "SpotLights", nullptr },
            { "DirectionalLights", nullptr }
        },
        std::unique_ptr<vpr::DescriptorSet>{ nullptr }
    };

    /*
        ReductionParams{ uint }
        AABB[] ClusterAABBs
        AABB[] LightAABBs
        U_IMAGE_BUFFER r32ui PointLightIndexCounter;
        U_IMAGE_BUFFER r32ui SpotLightIndexCounter;
        U_IMAGE_BUFFER r32ui PointLightIndexList;
        U_IMAGE_BUFFER r32ui SpotLightIndexList;
        U_IMAGE_BUFFER rg32ui PointLightGrid;
        U_IMAGE_BUFFER rg32ui SpotLightGrid;
    */

    resources_collection_t SortResources{
        std::unordered_map<std::string, std::unique_ptr<vpr::Buffer>> {
            { "ReductionParams", nullptr },
            { "ClusterAABBs", nullptr },
            { "LightAABBs", nullptr },
            { "PointLightIndexCounter", nullptr },
            { "SpotLightIndexCounter", nullptr }
        },
        std::unique_ptr<vpr::DescriptorSet>{ nullptr }
    };

    resources_collection_t MergeSortResources { 
        std::unordered_map<std::string, std::unique_ptr<vpr::Buffer>> {
            { "SortParams", nullptr },
            { "InputKeys", nullptr },
            { "InputValues", nullptr },
            { "OutputKeys", nullptr },
            { "OutputValues", nullptr },
            { "MergePathPartitions", nullptr }
        },
        std::unique_ptr<vpr::DescriptorSet>{ nullptr }
    };

    resources_collection_t BVHResources{
        std::unordered_map<std::string, std::unique_ptr<vpr::Buffer>> {
            { "BVHParams", nullptr },
            { "PointLightBVH", nullptr },
            { "SpotLightBVH", nullptr }
        },
        std::unique_ptr<vpr::DescriptorSet>{ nullptr }
    };

    resources_collection_t MortonResources{
        std::unordered_map<std::string, std::unique_ptr<vpr::Buffer>>{
            { "PointLightMortonCodes", nullptr },
            { "PointLightIndices", nullptr },
            { "SpotLightMortonCodes", nullptr },
            { "SpotLightIndices", nullptr }
        },
        std::unique_ptr<vpr::DescriptorSet>{ nullptr }
    };

    struct compute_pipeline_t {
        compute_pipeline_t() = default;
        ~compute_pipeline_t() = default;
        VkPipeline Handle{ VK_NULL_HANDLE };
        VkComputePipelineCreateInfo Info{ vpr::vk_compute_pipeline_create_info_base };
        std::unique_ptr<vpr::PipelineLayout> Layout{ nullptr };
        std::unique_ptr<vpr::PipelineCache> Cache{ nullptr };
    };

    struct compute_pipelines_t {
        std::unordered_map<std::string, compute_pipeline_t> Handles = std::unordered_map<std::string, compute_pipeline_t>{
            { "UpdateLights", compute_pipeline_t() },
            { "ReduceLights0", compute_pipeline_t() },
            { "ReduceLights1", compute_pipeline_t() },
            { "ComputeMortonCodes", compute_pipeline_t() },
            { "RadixSort", compute_pipeline_t() },
            { "MergePathPartitions", compute_pipeline_t() },
            { "MergeSort", compute_pipeline_t() },
            { "BuildBVH_Bottom", compute_pipeline_t() },
            { "BuildBVH_Top", compute_pipeline_t() },
            { "ComputeGridFrustums", compute_pipeline_t() },
            { "ComputeClusterAABBs", compute_pipeline_t() },
            { "IndirectArgsUpdate", compute_pipeline_t() },
            { "AssignLightsToClusters", compute_pipeline_t() }
        };

        compute_pipeline_t& at(const std::string& key);
    } ComputePipelines;

    constexpr static uint32_t SORTING_NUM_THREADS_PER_THREAD_GROUP = 256;
    constexpr static uint32_t SORT_NUM_VALUES_PER_THREAD = 8;

    void MergeSort(const VkCommandBuffer& cmd, vpr::Buffer* src_keys, vpr::Buffer* src_values, vpr::Buffer* dst_keys, 
        vpr::Buffer* dst_values, const uint32_t num_values, const uint32_t chunk_size) {
        
        auto& merge_path_partitions = MergeSortResources.Resources.at("MergePathPartitions");
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
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, merge_partitions_pipeline.Layout->vkHandle(), 0, 1, &MergeSortResources.Descriptor->vkHandle(), 0, nullptr);
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

        if (pass % 2 == 1) {

        }
    }

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

    void UpdateLights(const VkCommandBuffer& cmd) {
        auto& update_pipeline = ComputePipelines.at("UpdateLights");
        VkDescriptorSet sets[2]{ LightResources.Descriptor->vkHandle(), SortResources.Descriptor->vkHandle() };
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, update_pipeline.Handle);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, update_pipeline.Layout->vkHandle(), 0, 2, sets, 0, nullptr);
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
        SortResources.Resources.at("DispatchParams")->Update(cmd, sizeof(dispatch_params_t), 0, &params0);
        vkCmdDispatch(cmd, num_thread_groups, 1, 1);
        dispatch_params_t params1;
        params1.NumThreadGroups = glm::uvec3(1, 1, 1);
        params1.NumThreads = glm::uvec3(512, 1, 1);
        SortResources.Resources.at("DispatchParams")->Update(cmd, sizeof(dispatch_params_t), 0, &params1);
        vkCmdDispatch(cmd, 1, 1, 1);    
    }

    void ComputeMortonCodes(const VkCommandBuffer& cmd) {
        auto& morton_pipeline = ComputePipelines.at("ComputeMortonCodes");
        // Only need to bind this single extra set
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, morton_pipeline.Layout->vkHandle(), 2, 1, &MortonResources.Descriptor->vkHandle(), 0, nullptr);
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
        ClusterResources.at("ClusterFlags")->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(uint8_t) * num_clusters);
        ClusterResources.at("ClusterFlags")->CreateView(VK_FORMAT_R8_UINT, ClusterResources.at("ClusterFlags")->Size(), 0);
        ClusterResources.at("UniqueClusters")->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(uint32_t) * num_clusters);
        ClusterResources.at("UniqueClusters")->CreateView(VK_FORMAT_R32_UINT, ClusterResources.at("UniqueClusters")->Size(), 0);
        ClusterResources.at("PreviousUniqueClusters")->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(uint32_t) * num_clusters);
        ClusterResources.at("PreviousUniqueClusters")->CreateView(VK_FORMAT_R32_UINT, ClusterResources.at("PreviousUniqueClusters")->Size(), 0);

        // TODO: Generate lights, update clusters now. Need some kind of byte address buffer?
    }

    void ClusteredPrePass(const VkCommandBuffer& cmd) {
        ClusterResources.at("ClusterFlags")->Fill(cmd, ClusterResources.at("ClusterFlags")->Size(), 0, 0);

    }
}