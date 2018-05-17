#pragma once
#ifndef VPSK_MATERIAL_COMPONENT_HPP
#define VPSK_MATERIAL_COMPONENT_HPP
#include "glm/vec4.hpp"

namespace vpsk {

    class Texture;

    struct MaterialTextures {
        Texture* Diffuse{ nullptr };
        Texture* Normal{ nullptr };
        Texture* AmbientOcclusion{ nullptr };
        Texture* Roughness{ nullptr };
        Texture* Metallic{ nullptr };
        Texture* Emissive{ nullptr };
    };

    struct MaterialTextureFlags {
        bool Diffuse{ false };
        bool Normal{ false };
        bool AmbientOcclusion{ false };
        bool Roughness{ false };
        bool Metallic{ false };
        bool Emissive{ false };
    };

    struct MaterialParameters {
        glm::vec4 ambient{ 0.0f, 0.0f, 0.0f, 0.0f };
        glm::vec4 diffuse{ 0.0f, 0.0f, 0.0f, 0.0f };
        glm::vec4 specular{ 0.0f, 0.0f, 0.0f, 0.0f };
        glm::vec4 transmittance{ 0.0f, 0.0f, 0.0f, 0.0f };
        glm::vec4 emission{ 0.0f, 0.0f, 0.0f, 0.0f };
        float shininess{ 0.0f };
        float ior{ 0.0f };
        float alpha{ 0.0f };
        int illuminationModel{ 0 };
        float roughness{ 0.0f };
        float metallic{ 0.0f };
        float sheen{ 0.0f };
        float clearcoat_thickness{ 0.0f };
        float clearcoat_roughness{ 0.0f };
        float anisotropy{ 0.0f };
        float anisotropy_rotation{ 0.0f };
        float padding{ 0.0f };
    };

    struct MaterialComponent {
        MaterialTextures Textures;
        MaterialParameters Parameters;
        MaterialTextureFlags Flags;
    };

}

#endif // !VPSK_MATERIAL_COMPONENT_HPP
