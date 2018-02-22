#pragma once
#ifndef VULPESRENDER_MATERIALS_HPP
#define VULPESRENDER_MATERIALS_HPP
#include "ForwardDecl.hpp"
#include "resource/Texture.hpp"
#include <list>
#include <memory>
#include "glm/vec4.hpp"

namespace vpsk {

    struct pbrTexturePack;
    
    /** Hides away the tremendous amount of textures and data required to render more complicated .obj models.
    *   Uses tinyobj to load the obj file, and enables all of the features it can. Can also be passed an mtl file path,
    *   otherwise accepts a tinyobj::material_t reference that it will use for loading the textures.
    *   
    *   When available, it will also load and create the textures required for PBR rendering. Note that shaders must still
    *   be setup to use the available textures, and a descriptor set with enough VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER objects
    *   allocated must also exist (and be passed to the Create method).
    *   \ingroup Objects
    */
    class Material {
    
        struct material_ubo_t {
            glm::vec4 ambient = glm::vec4(0.0f);
            glm::vec4 diffuse = glm::vec4(0.0f);
            glm::vec4 specular = glm::vec4(0.0f);
            glm::vec4 transmittance = glm::vec4(0.0f);
            glm::vec4 emission = glm::vec4(0.0f);
            struct misc_info_t {
                float shininess = 0.0f;
                float indexOfRefraction = 0.0f;
                float dissolve = 0.0f;
                int illuminationModel = 0;
            } miscInfo;
        };

        Material(const Material&) = delete;
        Material& operator=(const Material&) = delete;

    public:

        Material() = default;
        ~Material() = default;

        Material(Material&&) noexcept;
        Material& operator=(Material&&) noexcept;

        
        void UploadToDevice(vpr::TransferPool* transfer_pool);

        /** Binds the descriptor set belonging to this object, to which all available textures will have been attached.
        *   This is the only required command during rendering to use the Material class.
        */
        void BindMaterial(const VkCommandBuffer& cmd, const VkPipelineLayout& pipeline_layout) const noexcept;

        VkDescriptorSetLayout GetSetLayout() const noexcept;
        VkDescriptorSetLayout GetPbrSetLayout() const noexcept;

    private:

        void createHashCode();
       
    
        std::unique_ptr<vpr::Texture<vpr::texture_2d_t>> ambient = nullptr;
        std::unique_ptr<vpr::Texture<vpr::texture_2d_t>> diffuse = nullptr;
        std::unique_ptr<vpr::Texture<vpr::texture_2d_t>> specular = nullptr;
        std::unique_ptr<vpr::Texture<vpr::texture_2d_t>> specularHighlight = nullptr;
        std::unique_ptr<vpr::Texture<vpr::texture_2d_t>> bumpMap = nullptr;
        std::unique_ptr<vpr::Texture<vpr::texture_2d_t>> displacementMap = nullptr;
        std::unique_ptr<vpr::Texture<vpr::texture_2d_t>> alpha = nullptr;
        std::unique_ptr<vpr::Texture<vpr::texture_2d_t>> reflection = nullptr;

        std::unique_ptr<vpr::Buffer> ubo = nullptr;
        material_ubo_t uboData;
        std::unique_ptr<vpr::DescriptorSet> descriptorSet;
        std::unique_ptr<vpr::DescriptorSetLayout> setLayout;
        // OPTIONAL
        std::unique_ptr<pbrTexturePack> pbrTextures;

        const vpr::Device* device;
        uint64_t hashCode;
        std::list<vpr::Texture<vpr::texture_2d_t>*> activeTextures;

    };

    struct pbrTexturePack {

        struct pbr_ubo_t {
            float roughness;
            float metallic;
            float sheen;
            float clearcoat_thickness;
            float clearcoat_roughness;
            float anisotropy;
            float anisotropy_rotation;
            const char padding[4]{ 0, 0, 0, 0 };
        };

        pbrTexturePack() = default;
        ~pbrTexturePack() = default;

        std::unique_ptr<vpr::Texture<vpr::texture_2d_t>> Roughness = nullptr;
        std::unique_ptr<vpr::Texture<vpr::texture_2d_t>> Metallic = nullptr;
        std::unique_ptr<vpr::Texture<vpr::texture_2d_t>> Sheen = nullptr;
        std::unique_ptr<vpr::Texture<vpr::texture_2d_t>> Emissive = nullptr;
        std::unique_ptr<vpr::Texture<vpr::texture_2d_t>> NormalMap = nullptr;
        std::unique_ptr<vpr::Buffer> ubo = nullptr;
        pbr_ubo_t uboData;

        std::unique_ptr<vpr::DescriptorSet> descriptorSet;
        std::unique_ptr<vpr::DescriptorSetLayout> setLayout;

    };

}

#endif //!VULPESRENDER_MATERIALS_HPP