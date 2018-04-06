#include "scene/BaseScene.hpp"
#include "common/CreateInfoBase.hpp"
#include "core/Instance.hpp"
#include "core/LogicalDevice.hpp"
#include "resource/DescriptorPool.hpp"
#include "resource/Buffer.hpp"
#include "util/UtilitySphere.hpp" 
#include "math/Ray.hpp"
#include "util/AABB.hpp"
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
#include "resource/Image.hpp"
#include "resources/ShaderGroup.hpp"
#include "scene/Window.hpp"
#include "camera/Camera.hpp"
#include "ShaderGenerator.hpp"
#include <memory>
#include <set>
#include <unordered_map>
#include <map>
#include <random>
#include <deque>
#include <array>
#include <experimental/filesystem>
#include <variant>
#include <future>
#include <mutex>

#include "util/easylogging++.h"
INITIALIZE_EASYLOGGINGPP

namespace rendergraph_sketch {

    using backing_resource_t = std::variant<
        std::unique_ptr<vpr::Buffer>,
        std::unique_ptr<vpr::Image>
    >;

    using backing_resource_map_t = std::map<
        std::string,
        backing_resource_t
    >;

    struct compute_pipeline_t {
        VkPipeline Handle;
        VkComputePipelineCreateInfo CreateInfo;
    };

    struct render_pipeline_t {
        std::unique_ptr<vpr::GraphicsPipelineInfo> Info;
        std::unique_ptr<vpr::GraphicsPipeline> Pipeline;
    };

    using pipelines_variant_t = std::variant<
        compute_pipeline_t,
        render_pipeline_t
    >;

    struct descriptor_resources_library_t {
        size_t AddResources(const std::map<std::string, VkDescriptorSetLayoutBinding>& names);
        size_t FindIdxOfSetWithResource(const std::string& name) const;
        void AllocatePool(const vpr::Device* dvc);
        void CreateSetLayouts(const vpr::Device* dvc);
        std::vector<std::map<std::string, VkDescriptorSetLayoutBinding>> setResources;
        std::vector<std::unique_ptr<vpr::DescriptorSetLayout>> resourceLayouts;
        std::vector<std::unique_ptr<vpr::DescriptorSet>> descriptorSets;
        std::unique_ptr<vpr::DescriptorPool> descriptorPool;
        std::mutex insertionGuard;
    } ResourceLibrary;

    size_t descriptor_resources_library_t::AddResources(const std::map<std::string, VkDescriptorSetLayoutBinding>& names) {

        std::vector<std::future<size_t>> set_indices;
        for (const auto& name : names) {
            set_indices.emplace_back(std::async(std::launch::async, &descriptor_resources_library_t::FindIdxOfSetWithResource, this, name.first));
        }

        std::vector<size_t> search_results;
        for (auto&& fut : set_indices) {
            search_results.emplace_back(fut.get());
        }

        auto valid_idx = [](const size_t& idx) { return idx != std::numeric_limits<size_t>::max(); };
        auto iter  = std::find_if(search_results.cbegin(), search_results.cend(), valid_idx);

        std::lock_guard<std::mutex> insertion_lock(insertionGuard);
        if (iter == search_results.cend()) {
            setResources.emplace_back(std::map<std::string, VkDescriptorSetLayoutBinding>{ names.cbegin(), names.cend() });
            return setResources.size() - 1;
        }
        else {
            size_t set_to_add_names_to = *iter;
            setResources[set_to_add_names_to].insert(names.cbegin(), names.cend());
            return set_to_add_names_to;
        }

    }

    size_t descriptor_resources_library_t::FindIdxOfSetWithResource(const std::string& name) const {
        
        auto has_resource = [name](const std::map<std::string, VkDescriptorSetLayoutBinding>& set_to_search)->bool {
            return (set_to_search.count(name) != 0);
        };

        std::vector<std::future<bool>> search_futures;
        for (const auto& set : setResources) {
            search_futures.emplace_back(std::async(std::launch::async, has_resource, set));
        }

        std::vector<bool> search_results;
        for (auto&& fut : search_futures) {
            search_results.emplace_back(fut.get());
        }

        auto iter = std::find(search_results.cbegin(), search_results.cend(), true);
        if (iter == search_results.cend()) {
            return std::numeric_limits<size_t>::max();
        }
        else {
            return static_cast<size_t>(std::distance(search_results.cbegin(), iter));
        }

    }

    void descriptor_resources_library_t::AllocatePool(const vpr::Device * dvc) {
        descriptorPool = std::make_unique<vpr::DescriptorPool>(dvc, setResources.size());
        std::map<VkDescriptorType, uint32_t> num_per_type;

        for (const auto& entry : setResources) {
            for (auto& rsrc : entry) {
                num_per_type[rsrc.second.descriptorType] += rsrc.second.descriptorCount;
            }
        }

        for (auto& type : num_per_type) {
            descriptorPool->AddResourceType(type.first, type.second);
        }

        descriptorPool->Create();
    }

    void descriptor_resources_library_t::CreateSetLayouts(const vpr::Device * dvc) {
        for (const auto& entry : setResources) {
            std::unique_ptr<vpr::DescriptorSetLayout> set_layout(std::make_unique<vpr::DescriptorSetLayout>(dvc));
            std::vector<VkDescriptorSetLayoutBinding> bindings;
            for (const auto& rsrc : entry) {
                bindings.emplace_back(rsrc.second);
            }
            set_layout->AddDescriptorBindings(bindings);
            resourceLayouts.emplace_back(std::move(set_layout));
        }
    }

    struct base_pass_t {
        base_pass_t(const vpr::Device* dvc, const std::vector<std::string>& shader_paths);
        base_pass_t(const vpr::Device* dvc, const std::vector<std::string>& shader_names, const std::vector<std::string>& shader_src_strings);
        pipelines_variant_t Pipeline;
        std::vector<size_t> ResourceLibraryIndices;
        std::unique_ptr<vpsk::ShaderGroup> Shaders;
        std::unique_ptr<vpr::PipelineLayout> Layout;
        std::unique_ptr<vpr::PipelineCache> Cache;
        const vpr::Device* device;
        bool operator==(const base_pass_t& other);
    };

    base_pass_t::base_pass_t(const vpr::Device* dvc, const std::vector<std::string>& paths) : device(dvc), 
        Shaders(std::make_unique<vpsk::ShaderGroup>(dvc)) {
        namespace fs = std::experimental::filesystem;
        bool is_compute_pass = false;

        if (paths.size() == 1) {
            fs::path path(paths.front());
            std::string extension = path.extension().string();
            if (extension != ".comp") {
                std::cerr << "Only one shader passed to base_pass_t ctor, when more than one is needed for non-compute passes!";
            }
            Shaders->AddShader(paths.front().c_str(), VK_SHADER_STAGE_COMPUTE_BIT);
            is_compute_pass = true;
        }
        else {
            for (auto& shader : paths) {
                Shaders->AddShader(shader.c_str());
            }
        }

        uint32_t num_sets = static_cast<uint32_t>(Shaders->GetNumSetsRequired());
        for(uint32_t i = 0; i < num_sets; ++i) {
            ResourceLibraryIndices.emplace_back(ResourceLibrary.AddResources(Shaders->GetSetLayoutBindings(i)));
        }

    }

    base_pass_t::base_pass_t(const vpr::Device * dvc, const std::vector<std::string>& shader_names, const std::vector<std::string>& shader_src_strings) : device(dvc),
        Shaders(std::make_unique<vpsk::ShaderGroup>(dvc)) {
        if (shader_names.size() == 1) {
            Shaders->AddShader(shader_names.front(), shader_src_strings.front(), VK_SHADER_STAGE_COMPUTE_BIT);
        }
        uint32_t num_sets = static_cast<uint32_t>(Shaders->GetNumSetsRequired());
        for (uint32_t i = 0; i < num_sets; ++i) {
            ResourceLibraryIndices.emplace_back(ResourceLibrary.AddResources(Shaders->GetSetLayoutBindings(i)));
        }
    }

    class RenderGraph {

    public:

        std::unordered_map<std::string, base_pass_t> passes;
        std::vector<std::unique_ptr<vpr::DescriptorSet>> descriptorSets;
        std::vector<std::unique_ptr<vpr::DescriptorSetLayout>> setLayouts;
        std::unique_ptr<vpr::DescriptorPool> descriptorPool;
    };

}

constexpr VkApplicationInfo app_info{
    VK_STRUCTURE_TYPE_APPLICATION_INFO,
    nullptr,
    "RendergraphSketch",
    VK_MAKE_VERSION(0,1,0),
    "VulpesSceneKit",
    VK_MAKE_VERSION(0,1,0),
    VK_API_VERSION_1_1
};

int main(int argc, char* argv[]) {
    using namespace rendergraph_sketch;
    ImGui::CreateContext();
    auto window = std::make_unique<vpsk::Window>(1280, 720, "RendergraphSketch");
    auto instance = std::make_unique<vpr::Instance>(false, &app_info, window->glfwWindow());
    auto device = std::make_unique<vpr::Device>(instance.get(), instance->GetPhysicalDevice(), true);

    std::map<std::string, std::string> generatedShaders;
    {
        std::vector<std::string> compute_shader_names {
            "AssignLightsToClustersBVH",
            "BuildBVH",
            "ComputeMortonCodes",
            "FindUniqueClusters",
            "MergeSort",
            "RadixSort",
            "ReduceLightsAABB",
            "UpdateClusterIndirectArgs",
            "UpdateLights"
        };

        const std::string path_prefix{ "../third_party/shadertools/fragments/volumetric_forward/" };
        std::string base_path{ "../third_party/shadertools/fragments/" };
        st::ShaderGenerator::SetBasePath(base_path.c_str());
        for (auto& name : compute_shader_names) {
            st::ShaderGenerator gen(VK_SHADER_STAGE_COMPUTE_BIT);
            gen.AddResources("../third_party/shadertools/fragments/volumetric_forward/Resources.glsl");
            std::array<const char*, 1> includes{ path_prefix.c_str() };
            std::string path_to_src = path_prefix + std::string("compute/") + name + std::string(".comp");
            gen.AddBody(path_to_src.c_str(), includes.size(), includes.data());
            size_t len = 0;
            gen.GetFullSource(&len, nullptr);
            std::vector<char> generated_src(len);
            gen.GetFullSource(&len, generated_src.data());
            generatedShaders.emplace(name, std::string{ generated_src.cbegin(), generated_src.cend() });
        }

    }
    
    std::map<std::string, base_pass_t> passes;

    for (auto& shader : generatedShaders) {
        passes.emplace(shader.first, base_pass_t(device.get(), { shader.first }, { shader.second }));
    }

    ResourceLibrary.AllocatePool(device.get());
    ResourceLibrary.CreateSetLayouts(device.get());

    std::cerr << "Completed " << std::to_string(ResourceLibrary.setResources.size()) << "\n";
    
}