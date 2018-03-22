#pragma once
#ifndef VPSK_SHADER_GROUP_HPP
#define VPSK_SHADER_GROUP_HPP
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <map>
#include <memory>

namespace st {
    class BindingGenerator;
    class ShaderCompiler;
}

namespace vpsk {

    /*  Designed to be used to group shaders into the groups that they are used in
        when bound to a pipeline, to simplify a few key things.
    */
    class ShaderGroup {
    public:

        ShaderGroup();
        ~ShaderGroup();

        void AddShader(const char* fname, const VkShaderStageFlagBits stage);
        void AddShader(const std::string& full_shader_str, const VkShaderStageFlagBits stage);

        const std::vector<VkVertexInputAttributeDescription>& GetAttributesForStage(const VkShaderStageFlagBits stage) const;
        const std::vector<VkDescriptorSetLayoutBinding>& GetBindingsForStage(const VkShaderStageFlagBits stage) const;
        std::vector<VkPipelineShaderStageCreateInfo> GetPipelineInfos() const;
        
    private:
        
        std::unique_ptr<st::BindingGenerator> bindingGen;
        std::unique_ptr<st::ShaderCompiler> compiler;
        std::map<VkShaderStageFlagBits, std::vector<uint32_t>> binaries;
    };

}

#endif //!VPSK_SHADER_GROUP_HPP