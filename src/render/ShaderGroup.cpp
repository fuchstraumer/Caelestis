#include "render/ShaderGroup.hpp"

namespace vpsk {
    
    void ShaderGroup::AddShader(const std::string& raw_path, const VkShaderStageFlags& stage) {
        if(compiler.HasShader(raw_path)) {
            bindings.ParseBinary(compiler.GetBinary(raw_path), stage);
        }
        else {
            compiler.Compile(raw_path, stage);
            bindings.ParseBinary(compiler.GetBinary(raw_path), stage);
        }
    }

    const std::vector<uint32_t>& ShaderGroup::CompileAndAddShader(const std::string& raw_path, const VkShaderStageFlags& stage) {
        const auto& result = compiler.Compile(raw_path, stage);
        AddShader(raw_path, stage);
        return result;
    }

}