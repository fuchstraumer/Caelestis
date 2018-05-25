#pragma once
#ifndef VPSK_MATERIAL_RESOURCE_HPP
#define VPSK_MATERIAL_RESOURCE_HPP
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <string>
#include <memory>
#include "glm/vec4.hpp"

namespace vpsk {

    class Texture;
    
    struct material_textures_t {
        Texture* Diffuse{ nullptr };
        Texture* Normal{ nullptr };
        Texture* AmbientOcclusion{ nullptr };
        Texture* Roughness{ nullptr };
        Texture* Metallic{ nullptr };
        Texture* Emissive{ nullptr };
        bool operator==(const material_textures_t& other) const noexcept {
            return (Diffuse == other.Diffuse) && (Normal == other.Normal) &&
                (AmbientOcclusion == other.AmbientOcclusion) && (Roughness == other.Roughness) &&
                (Metallic == other.Metallic) && (Emissive == other.Emissive);
        }
    };

    struct material_texture_flags_t {
        // Using VkBool32 as we may end up directly using
        // this struct as part of our data pointer
        VkBool32 Diffuse{ VK_FALSE };
        VkBool32 Normal{ VK_FALSE };
        VkBool32 AmbientOcclusion{ VK_FALSE };
        VkBool32 Roughness{ VK_FALSE };
        VkBool32 Metallic{ VK_FALSE };
        VkBool32 Emissive{ VK_FALSE };
        bool operator==(const material_texture_flags_t& other) const noexcept {
            return (Diffuse == other.Diffuse) && (Normal == other.Normal) &&
                (AmbientOcclusion == other.AmbientOcclusion) && (Roughness == other.Roughness) &&
                (Metallic == other.Metallic) && (Emissive == other.Emissive);
        }
    };

    struct material_parameters_t {
        glm::vec4 ambient{ 0.0f, 0.0f, 0.0f, 0.0f };
        glm::vec4 diffuse{ 0.0f, 0.0f, 0.0f, 0.0f };
        glm::vec4 specular{ 0.0f, 0.0f, 0.0f, 0.0f };
        glm::vec4 transmittance{ 0.0f, 0.0f, 0.0f, 0.0f };
        glm::vec4 emission{ 0.0f, 0.0f, 0.0f, 0.0f };
        float shininess{ 0.0f };
        float alpha{ 0.0f };
        float roughness{ 0.0f };
        float metallic{ 0.0f };
        bool operator==(const material_parameters_t& other) const noexcept {
            return (ambient == other.ambient) && (diffuse == other.diffuse) &&
                (specular == other.specular) && (transmittance == other.transmittance) &&
                (emission == other.emission) && (shininess == other.shininess) &&
                (alpha == other.alpha) && (roughness == other.roughness) && 
                (metallic == other.metallic);
        }
    };

    class Material {
    public:

        material_textures_t& Textures() noexcept;
        const material_textures_t& Textures() const noexcept;
        material_texture_flags_t& Flags() noexcept;
        const material_texture_flags_t& Flags() const noexcept;
        material_parameters_t& Parameters() noexcept;
        const material_parameters_t& Parameters() const noexcept;

    private:
        material_textures_t texturePtrs;
        material_texture_flags_t textureFlags;
        material_parameters_t parameters;
    };

}

#endif //!VPSK_MATERIAL_RESOURCE_HPP