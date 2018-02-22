#pragma once
#ifndef VPSK_TEXTURE_POOL_HPP
#define VPSK_TEXTURE_POOL_HPP
#include "ForwardDecl.hpp"
#include "resource/Texture.hpp"
#include "tinyobjloader/tiny_obj_loader.h"
#include "gli/gli.hpp"
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <unordered_set>

namespace vpsk {

    struct misc_material_data_t {
        float shininess;
        float ior;
        float alpha;
        int illum;
    };

    struct pbr_material_data_t {
        float roughness;
        float metallic;
        float sheen;
        float clearcoat_thickness;
        float clearcoat_roughness;
        float anisotropy;
        float anisotropy_rotation;
        float padding = 0.0f;
    };

    struct material_ubo_data_t {
        glm::vec4 ambient;
        glm::vec4 diffuse;
        glm::vec4 specular;
        glm::vec4 transmittance;
        glm::vec4 emission;
        misc_material_data_t data;
        pbr_material_data_t pbrData;
    };


    class TexturePool {
        TexturePool(const TexturePool&) = delete;
        TexturePool& operator=(const TexturePool&) = delete;
    public:

        TexturePool(const vpr::Device* dvc, const vpr::TransferPool* transfer_pool);
        ~TexturePool() = default;

        void AddMaterials(const std::vector<tinyobj::material_t>& material, const std::string& path_prefix);
        void BindMaterialAtIdx(const size_t & idx, const VkCommandBuffer cmd, const VkPipelineLayout layout, const size_t num_sets_prev_bound);
        const material_ubo_data_t& GetMaterialUBO(const size_t& idx) const;
        const VkDescriptorSetLayout GetSetLayout() const noexcept;

    private:

        void createDescriptorPool();
        void createDescriptorSetLayout();
        void createDescriptorSets();

        std::map<size_t, std::string> idxNameMap;
        std::unordered_map<std::string, material_ubo_data_t> materialUboData;
        std::unordered_map<std::string, std::unique_ptr<vpr::Buffer>> materialBuffers;
        std::unordered_map<std::string, std::unique_ptr<vpr::Texture<vpr::texture_2d_t>>> stbTextures;
        struct material_texture_data_t {
            decltype(stbTextures)::const_iterator Ambient;
            decltype(stbTextures)::const_iterator Diffuse;
            decltype(stbTextures)::const_iterator Specular;
            decltype(stbTextures)::const_iterator SpecularHighlight;
            decltype(stbTextures)::const_iterator Bump;
            decltype(stbTextures)::const_iterator Displacement;
            decltype(stbTextures)::const_iterator Alpha;
            decltype(stbTextures)::const_iterator Reflection;
            decltype(stbTextures)::const_iterator Roughness;
            decltype(stbTextures)::const_iterator Metallic;
            decltype(stbTextures)::const_iterator Sheen;
            decltype(stbTextures)::const_iterator Emissive;
            decltype(stbTextures)::const_iterator Normal;

        };
        std::unordered_map<std::string, material_texture_data_t> materialTextures;

        const vpr::TransferPool* transferPool;
        const vpr::Device* device;

        std::unique_ptr<vpr::DescriptorPool> descriptorPool;
        std::unique_ptr<vpr::DescriptorSetLayout> setLayout;
        std::map<std::string, std::unique_ptr<vpr::DescriptorSet>> materialSets;
    };

}

#endif //!VPSK_TEXTURE_POOL_HPP