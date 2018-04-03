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
#include <memory>
#include <map>
#include <random>
#include <deque>
#include <experimental/filesystem>

#include "util/easylogging++.h"
INITIALIZE_EASYLOGGINGPP
namespace volumetric_forward {

    constexpr uint32_t LightGridBlockSize = 32;
    constexpr uint32_t ClusterGridBlockSize = 64;

    struct render_target_t {
        render_target_t() = default;
        ~render_target_t() = default;
        void CreateRT(const vpr::Device* dvc, const vpr::Swapchain* swap, const VkRenderPass& pass, const VkImageUsageFlagBits usage, 
            const VkFormat fmt);

        std::unique_ptr<vpr::Image> Image{ nullptr };
        std::unique_ptr<vpr::Framebuffer> Framebuffer{ nullptr };
    } DepthOnlyRenderTarget;

    struct resources_collection_t {
        std::unordered_map<std::string, std::unique_ptr<vpr::Buffer>> Resources;
        std::unique_ptr<vpr::DescriptorSet> Descriptor;
    };

    /*
        U_IMAGE_BUFFER r32ui PointLightIndexCounter;
        U_IMAGE_BUFFER r32ui SpotLightIndexCounter;
        U_IMAGE_BUFFER r32ui PointLightIndexList;
        U_IMAGE_BUFFER r32ui SpotLightIndexList;
        U_IMAGE_BUFFER rg32ui PointLightGrid;
        U_IMAGE_BUFFER rg32ui SpotLightGrid;
        U_IMAGE_BUFFER r32ui InputKeys;
        U_IMAGE_BUFFER r32ui InputValues;
        U_IMAGE_BUFFER r32ui OutputKeys;
        U_IMAGE_BUFFER r32ui OutputValues;
        I_IMAGE_BUFFER r32i MergePathPartitions;
    */

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
            { "ReduceLights1", compute_pipeline_t() },
            { "ReduceLights2", compute_pipeline_t() },
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

    struct alignas(4) SortParams {
        uint32_t NumElements;
        uint32_t ChunkSize;
    };
    


    constexpr static uint32_t SORTING_NUM_THREADS_PER_THREAD_GROUP = 256;
    constexpr static uint32_t SORT_NUM_VALUES_PER_THREAD = 8;

    void MergeSort(const VkCommandBuffer& cmd, vpr::Buffer* src_keys, vpr::Buffer* src_values, vpr::Buffer* dst_keys, 
        vpr::Buffer* dst_values, const uint32_t num_values, const uint32_t chunk_size) {
        
        auto& merge_path_partitions = MergeSortResources.Resources.at("MergePathPartitions");
        auto& merge_pipeline = ComputePipelines.at("MergePathPartitions");

        constexpr uint32_t numThreadsPerGroup = SORTING_NUM_THREADS_PER_THREAD_GROUP;
        constexpr uint32_t numValuesPerThread = SORT_NUM_VALUES_PER_THREAD;
        constexpr uint32_t numValuesPerThreadGroup = numThreadsPerGroup * numValuesPerThread;

        uint32_t num_chunks = static_cast<uint32_t>(std::ceil(static_cast<double>(num_values) / static_cast<double>(chunk_size)));
        uint32_t pass = 0;
        uint32_t chunk_size_local = chunk_size;

        while (num_chunks > 1) {

            const SortParams params{ num_values, chunk_size_local };
            const uint32_t num_sort_groups = num_chunks / 2;
            const uint32_t num_thread_groups_per_sort_group = static_cast<uint32_t>(
                std::ceil(static_cast<double>(chunk_size * 2) / static_cast<double>(numValuesPerThreadGroup))
            );

            {
                merge_path_partitions->Fill(cmd, merge_path_partitions->Size(), 0, 0);
                vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, ComputePipelines.at("MergePathPartitions").Handle);
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, merge_pipeline.Layout->vkHandle(),
                    0, 1, &MergeSortResources.Descriptor->vkHandle(), 0, nullptr);
            }

        }
    }
}