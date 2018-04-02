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
#include "resources/ShaderGroup.hpp"
#include <memory>
#include <map>
#include <random>
#include <deque>
#include <experimental/filesystem>

#include "ShaderGenerator.hpp"
#include "Compiler.hpp"
#include "BindingGenerator.hpp"
#include "mango/mango/mango.hpp"

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

    struct compute_pipeline_t {
        VkPipeline Handle{ VK_NULL_HANDLE };
        VkComputePipelineCreateInfo Info{ vpr::vk_compute_pipeline_create_info_base };
        std::unique_ptr<vpr::PipelineCache> Cache{ nullptr };
    };

    struct compute_pipelines_t {
        std::unordered_map<std::string, compute_pipeline_t> Handles{
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
    } ComputePipelines;
    
    
}