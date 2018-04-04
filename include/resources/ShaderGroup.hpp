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
        ShaderGroup(const ShaderGroup&) = delete;
        ShaderGroup& operator=(const ShaderGroup&) = delete;
    public:

        ShaderGroup(const vpr::Device* dvc, st::ShaderCompiler* _compiler, st::BindingGenerator* binding_gen);
        ~ShaderGroup();
        ShaderGroup(ShaderGroup&& other) noexcept;
        ShaderGroup& operator=(ShaderGroup&& other) noexcept;

        void AddShader(const char* fname, VkShaderStageFlagBits stage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM);
        void AddShader(const std::string& shader_name, const std::string& src_str, const VkShaderStageFlagBits stage);

        const std::vector<VkVertexInputAttributeDescription>& GetVertexAttributes() const;
        const std::vector<VkDescriptorSetLayoutBinding>& GetSetLayoutBindings(const uint32_t set_idx) const;
        std::vector<VkPipelineShaderStageCreateInfo> GetPipelineInfos() const;
        size_t GetNumSetsRequired() const;
        
    private:

        void createModule(const st::Shader& handle);
        void retrieveData() const;

        const vpr::Device* device;
        mutable bool collated = false;

        std::map<VkShaderStageFlagBits, std::unique_ptr<vpr::ShaderModule>> shaders;
        std::map<VkShaderStageFlagBits, st::Shader> stHandles;

        mutable std::vector<VkVertexInputAttributeDescription> inputAttrs;
        mutable std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> layoutBindings;

        st::ShaderCompiler* compiler;
        st::BindingGenerator* bindingGenerator;
    };

}

#endif //!VPSK_SHADER_GROUP_HPP