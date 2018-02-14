#pragma once
#ifndef VPSK_TEXTURE_POOL_HPP
#define VPSK_TEXTURE_POOL_HPP
#include "ForwardDecl.hpp"
#include "resource/Texture.hpp"
#include "tiny_obj_loader.h"
#include "gli/gli.hpp"
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
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

    private:

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
    };

}

#endif //!VPSK_TEXTURE_POOL_HPP