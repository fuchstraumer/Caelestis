#include "resources/ShaderGroup.hpp"
#include "Compiler.hpp"
#include "BindingGenerator.hpp"
#include "core/LogicalDevice.hpp"
#include "resource/ShaderModule.hpp"
#include <fstream>
#include <experimental/filesystem>

namespace vpsk {

    static const std::map<std::string, VkShaderStageFlagBits> extension_stage_map {
        { ".vert", VK_SHADER_STAGE_VERTEX_BIT },
        { ".frag", VK_SHADER_STAGE_FRAGMENT_BIT },
        { ".geom", VK_SHADER_STAGE_GEOMETRY_BIT },
        { ".teval", VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT },
        { ".tcntl", VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT },
        { ".comp", VK_SHADER_STAGE_COMPUTE_BIT }
    };

    VkShaderStageFlagBits GetStageFromFilename(const char* fname) {
        namespace fs = std::experimental::filesystem;
        fs::path fpath(fname);
        if (!fs::exists(fpath)) {
            throw std::runtime_error("Found no file at given path, when trying to guess shader stage!");
        }

        const std::string fextension = fpath.extension().string();

        if (extension_stage_map.count(fextension) == 0) {
            throw std::runtime_error("Could not find shader stage for file with given extension!");
        }
        else {
            return extension_stage_map.at(fextension);
        }
    }

    ShaderGroup::ShaderGroup(const vpr::Device * dvc) : device(dvc), compiler(std::make_unique<st::ShaderCompiler>()), 
        bindingGenerator(std::make_unique<st::BindingGenerator>()) {}

    ShaderGroup::~ShaderGroup() {}

    ShaderGroup::ShaderGroup(ShaderGroup&& other) noexcept : device(std::move(other.device)), collated(std::move(other.collated)),
        shaders(std::move(other.shaders)), stHandles(std::move(other.stHandles)), inputAttrs(std::move(other.inputAttrs)),
        layoutBindings(std::move(other.layoutBindings)), bindingGenerator(std::move(other.bindingGenerator)), compiler(std::move(other.compiler)) {}

    ShaderGroup& ShaderGroup::operator=(ShaderGroup&& other) noexcept {
        device = std::move(other.device);
        collated = std::move(other.collated);
        shaders = std::move(other.shaders);
        stHandles = std::move(other.stHandles);
        inputAttrs = std::move(other.inputAttrs);
        layoutBindings = std::move(other.layoutBindings);
        bindingGenerator = std::move(other.bindingGenerator);
        compiler = std::move(other.compiler);
        return *this;
    }

    void ShaderGroup::AddShader(const char* fname, VkShaderStageFlagBits stage) {
        if (stage == VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM) {
            stage = GetStageFromFilename(fname);
        }
        
        st::Shader handle = compiler->Compile(fname, stage);
        bindingGenerator->ParseBinary(handle);
        stHandles.emplace(stage, handle);
        createModule(handle);
    }

    void ShaderGroup::AddShader(const std::string& shader_name, const std::string& shader_str, const VkShaderStageFlagBits stage) {
        st::Shader handle = compiler->Compile(shader_name.c_str(), shader_str.c_str(), shader_str.size(), stage);
        bindingGenerator->ParseBinary(handle);
        stHandles.emplace(stage, handle);
        createModule(handle);
    }

    const std::vector<VkVertexInputAttributeDescription>& ShaderGroup::GetVertexAttributes() const {
        if (!collated) {
            retrieveData();
        }
        return inputAttrs;
    }

    const std::map<std::string, VkDescriptorSetLayoutBinding>& ShaderGroup::GetSetLayoutBindings(const uint32_t set_idx) const {
        if (!collated) {
            retrieveData();
        }
        return layoutBindings.at(set_idx);
    }

    ShaderGroup::resources_tuple_t ShaderGroup::GetResources(const uint32_t set_idx) const {
        return std::make_tuple(GetSetLayoutBindings(set_idx), GetVertexAttributes());
    }

    std::vector<VkPipelineShaderStageCreateInfo> ShaderGroup::GetPipelineInfos() const {
        std::vector<VkPipelineShaderStageCreateInfo> results; results.reserve(shaders.size());
        for (auto& entry : shaders) {
            results.emplace_back(entry.second->PipelineInfo());
        }
        return results;
    }

    void ShaderGroup::createModule(const st::Shader & handle) {
        uint32_t sz = 0;
        compiler->GetBinary(handle, &sz, nullptr);
        std::vector<uint32_t> binary(sz);
        compiler->GetBinary(handle, &sz, binary.data());
        shaders.emplace(handle.GetStage(), std::make_unique<vpr::ShaderModule>(device, handle.GetStage(), binary.data(), sz));
    }

    size_t ShaderGroup::GetNumSetsRequired() const {
        if (!collated) {
            retrieveData();
        }
        return layoutBindings.size();
    }

    void ShaderGroup::retrieveData() const {
        collated = true;

        const uint32_t num_sets = bindingGenerator->GetNumSets();
        for (uint32_t i = 0; i < num_sets; ++i) {
            uint32_t num_names = 0;
            bindingGenerator->GetSetNameBindingPairs(i, &num_names, nullptr, nullptr);
            std::vector<char*> names(num_names);
            std::vector<VkDescriptorSetLayoutBinding> bindings(num_names);
            bindingGenerator->GetSetNameBindingPairs(i, &num_names, names.data(), bindings.data());

            for (size_t j = 0; j < num_names; ++j) {
                layoutBindings[i].emplace(std::string{ names[j] }, bindings[j]);
            }

            bindingGenerator->FreeRetrievedSetMemberNames(num_names, names.data());
            
        }

        uint32_t num_attrs = 0;
        bindingGenerator->GetVertexAttributes(&num_attrs, nullptr);
        if (num_attrs != 0) {
            inputAttrs = std::vector<VkVertexInputAttributeDescription>(num_attrs);
            bindingGenerator->GetVertexAttributes(&num_attrs, inputAttrs.data());
        }

        bindingGenerator.reset();
        compiler.reset();
        
    }

}