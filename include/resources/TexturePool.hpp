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

    using texture_pool_image_entry_t = std::map<std::string, VkDescriptorImageInfo>::const_iterator;
    using texture_pool_buffer_entry_t = std::map<std::string, VkDescriptorBufferInfo>::const_iterator;

    struct material_pool_entry_t {
        texture_pool_image_entry_t ambient;
        texture_pool_image_entry_t diffuse;
        texture_pool_image_entry_t specular;
        texture_pool_image_entry_t specularHighlight;
        texture_pool_image_entry_t bump;
        texture_pool_image_entry_t displacement;
        texture_pool_image_entry_t alpha;
        texture_pool_image_entry_t reflection;
        texture_pool_image_entry_t roughness;
        texture_pool_image_entry_t metallic;
        texture_pool_image_entry_t sheen;
        texture_pool_image_entry_t emissive;
        texture_pool_image_entry_t normal;
        texture_pool_buffer_entry_t buffer;
    };

    struct loaded_texture_t {
        loaded_texture_t(const loaded_texture_t&) = delete;
        loaded_texture_t& operator=(const loaded_texture_t&) = delete;
        loaded_texture_t(const std::string& path);
        loaded_texture_t(loaded_texture_t&& other) noexcept;
        loaded_texture_t(loaded_texture_t&& other) noexcept;
        VkExtent2D Extents;
        VkFormat Format;
        std::string Filename;
        stbi_uc* Pixels;
    };

    class TexturePool {
        TexturePool(const TexturePool&) = delete;
        TexturePool& operator=(const TexturePool&) = delete;
    public:

        TexturePool(const vpr::Device* dvc, const vpr::TransferPool* transfer_pool);
        ~TexturePool() = default;

        void AddMaterials(const std::vector<tinyobj::material_t>& material, const std::string& path_prefix);
        void AddTexture(const std::string& path);
        void AddTexture(const std::string& path, const VkFormat format);

        texture_pool_image_entry_t GetTextureDescriptor(const std::string& name);
        texture_pool_buffer_entry_t GetBufferDescriptor(const std::string& name);

        void BindMaterialAtIdx(const size_t& idx, const VkCommandBuffer cmd, const VkPipelineLayout layout);


    private:

        void createDescriptorPool();
        void createDescriptorSets();

        
        
        std::map<size_t, std::string> idxNameMap;

        std::unordered_map<std::string, VkDescriptorImageInfo> textureDescriptors;
        std::unordered_map<std::string, VkDescriptorBufferInfo> bufferDescriptors;

        std::unordered_map<std::string, material_ubo_data_t> materialUboData;
        std::unordered_map<std::string, std::unique_ptr<vpr::Buffer>> materialBuffers;
        std::unordered_map<std::string, std::unique_ptr<vpr::Texture<vpr::texture_2d_t>>> stbTextures;
        std::unordered_map<std::string, std::unique_ptr<vpr::Texture<gli::texture2d>>> gli2dTextures;
        // Uses mtl name to store iterators to all of it's textures.
        std::unordered_multimap<std::string, decltype(stbTextures)::iterator> materialTextures;
        std::unordered_map<std::string, std::unique_ptr<vpr::Texture<gli::texture_cube>>> gliCubeTextures;
        std::unordered_map<std::string, std::unique_ptr<vpr::Texture<gli::texture2d_array>>> gliArrayTextures;
        
        const vpr::TransferPool* transferPool;
        const vpr::Device* device;

        std::unique_ptr<vpr::DescriptorPool> descriptorPool;
        std::unordered_map<std::string, std::unique_ptr<vpr::DescriptorSet>> materialSets;
    };

}

namespace std {

    template<>
    struct hash<VkExtent2D> {
        size_t operator()(const VkExtent2D& ex) const {
            return std::hash<uint32_t>()(ex.width) ^ std::hash<uint32_t>()(ex.height);
        }
    };

    template<>
    struct hash<VkFormat> {
        size_t operator()(const VkFormat& format) const {
            return std::hash<uint32_t>()(static_cast<uint32_t>(format));
        }
    };

    template<>
    struct hash<vpsk::loaded_texture_t> {
        size_t operator()(const vpsk::loaded_texture_t& tex) const {
            return (std::hash<VkExtent2D>()(tex.Extents) << 16) ^ (std::hash<VkFormat>()(tex.Format));
        }
    };
}

#endif //!VPSK_TEXTURE_POOL_HPP