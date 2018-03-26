#include "resources/ShaderGroup.hpp"
#include "Compiler.hpp"
#include "BindingGenerator.hpp"
#include "core/LogicalDevice.hpp"
#include <fstream>

namespace vpsk {

    ShaderGroup::ShaderGroup(const vpr::Device * dvc) : device(dvc), compiler(std::make_unique<st::ShaderCompiler>()),
        bindingGen(std::make_unique<st::BindingGenerator>()) {}

    ShaderGroup::~ShaderGroup() {}

    void ShaderGroup::AddShader(const char* fname, const VkShaderStageFlagBits stage) {
        st::Shader handle = compiler->Compile(fname, stage);
        bindingGen->ParseBinary(handle);
        stHandles.emplace(stage, handle);
        createModule(handle);
    }

    void ShaderGroup::AddShader(const std::string& shader_name, const std::string& shader_str, const VkShaderStageFlagBits stage) {
        st::Shader handle = compiler->Compile(shader_name.c_str(), shader_str.c_str(), shader_str.size(), stage);
        bindingGen->ParseBinary(handle);
        stHandles.emplace(stage, handle);
        createModule(handle);
    }

    const std::vector<VkVertexInputAttributeDescription>& ShaderGroup::GetVertexAttributes() const {
        if (!collated) {
            retrieveData();
        }
        return inputAttrs;
    }

    const std::vector<VkDescriptorSetLayoutBinding>& ShaderGroup::GetSetLayoutBindings(const uint32_t set_idx) const {
        if (!collated) {
            retrieveData();
        }
        return layoutBindings.at(set_idx);
    }

    void ShaderGroup::createModule(const st::Shader & handle) {
        uint32_t sz = 0;
        compiler->GetBinary(handle, &sz, nullptr);
        std::vector<uint32_t> binary(sz);
        compiler->GetBinary(handle, &sz, binary.data());
        shaders.emplace(handle.GetStage(), std::make_unique<vpr::ShaderModule>(device, handle.GetStage(), binary.data(), sz));
    }

    void ShaderGroup::retrieveData() const {
        collated = true;

        bindingGen->CollateBindings();
        const uint32_t num_sets = bindingGen->GetNumSets();
        for (uint32_t i = 0; i < num_sets; ++i) {
            uint32_t num_bindings = 0;
            bindingGen->GetLayoutBindings(i, &num_bindings, nullptr);
            std::vector<VkDescriptorSetLayoutBinding> bindings(num_bindings);
            bindingGen->GetLayoutBindings(i, &num_bindings, bindings.data());
            layoutBindings.emplace(i, bindings);
        }

        uint32_t num_attrs = 0;
        bindingGen->GetVertexAttributes(&num_attrs, nullptr);
        inputAttrs = std::vector<VkVertexInputAttributeDescription>(num_attrs);
        bindingGen->GetVertexAttributes(&num_attrs, inputAttrs.data());
        
    }

}