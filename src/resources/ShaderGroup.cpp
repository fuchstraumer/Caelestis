#include "resources/ShaderGroup.hpp"
#include "Compiler.hpp"
#include "BindingGenerator.hpp"
#include <fstream>

namespace vpsk {

    ShaderGroup::ShaderGroup() : compiler(std::make_unique<st::ShaderCompiler>()), bindingGen(std::make_unique<st::BindingGenerator>()) {}

    ShaderGroup::~ShaderGroup() {}

    void ShaderGroup::AddShader(const char* fname, const VkShaderStageFlagBits stage) {
        std::ifstream input_file(fname);

        if (!input_file.is_open()) {
            throw std::runtime_error("Couldn't open shader file for reading!");
        }

        std::string shader_src_str{ std::istreambuf_iterator<char>(input_file), std::istreambuf_iterator<char>() };
        AddShader(shader_src_str, stage);

    }

    void ShaderGroup::AddShader(const std::string& shader_str, const VkShaderStageFlagBits stage) {

        compiler->Compile(shader_str.c_str(), shader_str.size(), stage);
        uint32_t binary_size = 0;
        compiler->GetBinary()
    }

}