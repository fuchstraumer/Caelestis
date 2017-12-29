#pragma once
#ifndef VPSK_SHADER_GROUP_HPP
#define VPSK_SHADER_GROUP_HPP
#include "ForwardDecl.hpp"
#include "shadertools/include/Compiler.hpp"
#include "shadertools/include/BindingGenerator.hpp"

namespace vpsk {

    class ShaderGroup {
    public:
        ShaderGroup(const vpr::Device* device);
        ~ShaderGroup() = default;

        void AddShader(const std::string& compiled_shader_path, const VkShaderStageFlags& stages);

        std::vector<std::vector<VkDescriptorSetLayoutBinding>> GetAllBindings() const;
        std::vector<VkPushConstantRange> GetPushConstants() const;
        std::vector<VkPipelineShaderStageCreateInfo> ShaderPipelineInfo() const;
        
    private:

        const std::vector<uint32_t>& compileAndAddShader(const std::string& uncompiled_shader_path, const VkShaderStageFlags& stages);

        const vpr::Device* dvc;
        std::vector<std::unique_ptr<vpr::ShaderModule>> modules;
        st::BindingGenerator bindings;
        static st::ShaderCompiler compiler;

    };

}
#endif //!VPSK_SHADER_GROUP_HPP