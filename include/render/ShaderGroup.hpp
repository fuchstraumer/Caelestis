#pragma once
#ifndef VPSK_SHADER_GROUP_HPP
#define VPSK_SHADER_GROUP_HPP
#include "shadertools/include/Compiler.hpp"
#include "shadertools/include/BindingGenerator.hpp"
namespace vpsk {

    class ShaderGroup {
    public:
        ShaderGroup() = default;
        ~ShaderGroup() = default;

        void AddShader(const std::string& compiled_shader_path, const VkShaderStageFlags& stages);
        const std::vector<uint32_t>& CompileAndAddShader(const std::string& uncompiled_shader_path, const VkShaderStageFlags& stages);

        std::vector<std::vector<VkDescriptorSetLayoutBinding>> GetAllBindings() const;
        std::vector<VkPushConstantRange> GetPushConstants() const;
        
    private:

        st::BindingGenerator bindings;
        static st::ShaderCompiler compiler;

    };

}
#endif //!VPSK_SHADER_GROUP_HPP