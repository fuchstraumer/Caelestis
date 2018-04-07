#include "common/CreateInfoBase.hpp"
#include "core/Instance.hpp"
#include "core/LogicalDevice.hpp"
#include "resources/ShaderGroup.hpp"
#include "scene/Window.hpp"
#include "ShaderGenerator.hpp"
#include "resources/DescriptorResources.hpp"
#include "resources/PipelineResource.hpp"
#include "render/PipelineSubmission.hpp"
#include "render/RenderGraph.hpp"
#include "imgui/imgui.h"
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

    std::unique_ptr<vpsk::DescriptorResources> ResourceLibrary;

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
            ResourceLibraryIndices.emplace_back(ResourceLibrary->AddResources(Shaders->GetSetLayoutBindings(i)));
        }

    }

    base_pass_t::base_pass_t(const vpr::Device * dvc, const std::vector<std::string>& shader_names, const std::vector<std::string>& shader_src_strings) : device(dvc),
        Shaders(std::make_unique<vpsk::ShaderGroup>(dvc)) {
        if (shader_names.size() == 1) {
            Shaders->AddShader(shader_names.front(), shader_src_strings.front(), VK_SHADER_STAGE_COMPUTE_BIT);
        }
        uint32_t num_sets = static_cast<uint32_t>(Shaders->GetNumSetsRequired());
        for (uint32_t i = 0; i < num_sets; ++i) {
            ResourceLibraryIndices.emplace_back(ResourceLibrary->AddResources(Shaders->GetSetLayoutBindings(i)));
        }
    }

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
    using namespace vpsk;
    using namespace vpr;
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

    RenderGraph graph(device.get());
    auto& AssignLightsToClustersBVH = graph.AddSubmission("AssignLightsToClustersBVH", VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    AssignLightsToClustersBVH.AddShaders({ "AssignLightsToClustersBVH" }, { generatedShaders.at("AssignLightsToClustersBVH") }, { VK_SHADER_STAGE_COMPUTE_BIT });
    

    std::cerr << "Completed\n";
    
}