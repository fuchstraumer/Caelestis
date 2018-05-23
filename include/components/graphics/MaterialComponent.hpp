#pragma once
#ifndef VPSK_MATERIAL_COMPONENT_HPP
#define VPSK_MATERIAL_COMPONENT_HPP
#include "glm/vec4.hpp"
#include <string>

namespace vpsk {

    class Texture;

    struct MaterialTextures {
        Texture* Diffuse{ nullptr };
        Texture* Normal{ nullptr };
        Texture* AmbientOcclusion{ nullptr };
        Texture* Roughness{ nullptr };
        Texture* Metallic{ nullptr };
        Texture* Emissive{ nullptr };
        bool operator==(const MaterialTextures& other) const noexcept {
            return (Diffuse == other.Diffuse) && (Normal == other.Normal) &&
                (AmbientOcclusion == other.AmbientOcclusion) && (Roughness == other.Roughness) &&
                (Metallic == other.Metallic) && (Emissive == other.Emissive);
        }
    };

    struct MaterialTextureFlags {
        bool Diffuse{ false };
        bool Normal{ false };
        bool AmbientOcclusion{ false };
        bool Roughness{ false };
        bool Metallic{ false };
        bool Emissive{ false };
        bool operator==(const MaterialTextureFlags& other) const noexcept {
            return (Diffuse == other.Diffuse) && (Normal == other.Normal) &&
                (AmbientOcclusion == other.AmbientOcclusion) && (Roughness == other.Roughness) &&
                (Metallic == other.Metallic) && (Emissive == other.Emissive);
        }
    };

    struct MaterialParameters {
        glm::vec4 ambient{ 0.0f, 0.0f, 0.0f, 0.0f };
        glm::vec4 diffuse{ 0.0f, 0.0f, 0.0f, 0.0f };
        glm::vec4 specular{ 0.0f, 0.0f, 0.0f, 0.0f };
        glm::vec4 transmittance{ 0.0f, 0.0f, 0.0f, 0.0f };
        glm::vec4 emission{ 0.0f, 0.0f, 0.0f, 0.0f };
        float shininess{ 0.0f };
        float alpha{ 0.0f };
        float roughness{ 0.0f };
        float metallic{ 0.0f };
        bool operator==(const MaterialParameters& other) const noexcept {
            return (ambient == other.ambient) && (diffuse == other.diffuse) &&
                (specular == other.specular) && (transmittance == other.transmittance) &&
                (emission == other.emission) && (shininess == other.shininess) &&
                (alpha == other.alpha) && (roughness == other.roughness) && 
                (metallic == other.metallic);
        }
    };

    struct MaterialComponent {
        MaterialTextures Textures;
        MaterialParameters Parameters;
        MaterialTextureFlags Flags;
        std::string Name;
        bool operator==(const MaterialComponent& other) const noexcept {
            return Name == other.Name;
        }
    };

}

#endif // !VPSK_MATERIAL_COMPONENT_HPP
