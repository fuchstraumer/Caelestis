#pragma once
#ifndef VPSK_MATERIAL_COMPONENT_HPP
#define VPSK_MATERIAL_COMPONENT_HPP
#include "glm/vec4.hpp"

namespace vpsk {

    class Texture;

    struct MaterialTextures {
        Texture* Ambient{ nullptr };
        Texture* Diffuse{ nullptr };
        Texture* Specular{ nullptr };
        Texture* SpecularHighlight{ nullptr };
        Texture* Bump{ nullptr };
        Texture* Displacement{ nullptr };
        Texture* Alpha{ nullptr };
        Texture* Reflection{ nullptr };
        Texture* Roughness{ nullptr };
        Texture* Metallic{ nullptr };
        Texture* Sheen{ nullptr };
        Texture* Emissive{ nullptr };
        Texture* Normal{ nullptr };
    };

    struct MaterialTextureFlags {
        bool Ambient{ false };
        bool Diffuse{ false };
        bool Specular{ false };
        bool SpecularHighlight{ false };
        bool Bump{ false };
        bool Displacement{ false };
        bool Alpha{ false };
        bool Reflection{ false };
        bool Roughness{ false };
        bool Metallic{ false };
        bool Sheen{ false };
        bool Emissive{ false };
        bool Normal{ false };
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
