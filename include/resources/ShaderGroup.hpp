#pragma once
#ifndef VPSK_SHADER_GROUP_HPP
#define VPSK_SHADER_GROUP_HPP
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include "ForwardDecl.hpp"
#include "Shader.hpp"
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

        ShaderGroup(const vpr::Device* dvc);
        ~ShaderGroup();

        void AddShader(const char* fname, const VkShaderStageFlagBits stage);
        void AddShader(const std::string& shader_name, const std::string& full_shader_str, const VkShaderStageFlagBits stage);

        const std::vector<VkVertexInputAttributeDescription>& GetVertexAttributes() const;
        const std::vector<VkDescriptorSetLayoutBinding>& GetSetLayoutBindings(const uint32_t set_idx) const;
        std::vector<VkPipelineShaderStageCreateInfo> GetPipelineInfos() const;
        
    private:

        void createModule(const st::Shader& handle);
        void retrieveData() const;

        const vpr::Device* device;
        mutable bool collated = false;

        std::map<VkShaderStageFlagBits, std::unique_ptr<vpr::ShaderModule>> shaders;
        std::map<VkShaderStageFlagBits, st::Shader> stHandles;

        mutable std::vector<VkVertexInputAttributeDescription> inputAttrs;
        mutable std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> layoutBindings;

        std::unique_ptr<st::BindingGenerator> bindingGen;
        std::unique_ptr<st::ShaderCompiler> compiler;
    };

}

#endif //!VPSK_SHADER_GROUP_HPP