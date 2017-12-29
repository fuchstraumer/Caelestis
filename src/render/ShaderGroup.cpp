#include "render/ShaderGroup.hpp"
#include "resource/ShaderModule.hpp"
namespace vpsk {

    st::ShaderCompiler ShaderGroup::compiler = st::ShaderCompiler();
    
    void ShaderGroup::AddShader(const std::string& raw_path, const VkShaderStageFlags& stage) {
        if(compiler.HasShader(raw_path)) {
            bindings.ParseBinary(compiler.GetBinary(raw_path), stage);
        }
        else {
            compileAndAddShader(raw_path, stage);
        }

        const auto& binary_code = compiler.GetBinary(raw_path);
        modules.emplace_back(std::make_unique<vpr::ShaderModule>(dvc, stage, binary_code.data(), static_cast<uint32_t>(binary_code.size())));

    }

    std::vector<VkPipelineShaderStageCreateInfo> ShaderGroup::ShaderPipelineInfo() const {
        std::vector<VkPipelineShaderStageCreateInfo> result;
        for(const auto& module : modules) {
            result.push_back(module->PipelineInfo());
        }
        return result;
    }

    const std::vector<uint32_t>& ShaderGroup::compileAndAddShader(const std::string& raw_path, const VkShaderStageFlags& stage) {
        const auto& result = compiler.Compile(raw_path, stage);
        AddShader(raw_path, stage);
        return result;
    }

}