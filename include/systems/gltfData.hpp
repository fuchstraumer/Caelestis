#pragma once
#ifndef VPSK_GLTF_DATA_HPP
#define VPSK_GLTF_DATA_HPP
#include "ForwardDecl.hpp"
#include "systems/ResourceLoader.hpp"
#include "resources/CommonStructs.hpp"
#include "resources/MeshData.hpp"
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <set>
#include <string>
#include <memory>

namespace fx {
    namespace gltf {
        struct Document;
    }
}

namespace vpsk {

    class Texture;

    struct gltfMaterial {
        std::string NormalTexture;
        std::string DiffuseTexture;
        std::string MetallicRoughnessTexture;
        std::string EmissiveTexture;
        std::string OcclusionTexture;

        enum class alpha_mode : uint32_t {
            Opaque, 
            Mask,
            Blend
        };

        struct parameters_t {
            float Diffuse[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
            float Metallic{ 1.0f };
            float Roughness{ 1.0f };
            float NormalScale{ 1.0f };
            float OcclusionStrength{ 1.0f };
            float EmissiveFactor[3]{ 0.0f, 0.0f, 0.0f };
            float AlphaCutoff{ 0.50f };
            VkBool32 DoubleSided{ VK_FALSE };
            alpha_mode AlphaMode{ alpha_mode::Opaque };
            float Padding[2]{ 0.0f, 0.0f };
        } Params;

        static_assert(sizeof(parameters_t) == 64, "Invalid parameters size");
    };

    struct gltfData : ResourceData {
        void Load(const LoadRequest& request) final;
        std::string FileName;
        std::string FilePath;
        std::unordered_map<std::string, vpr::Sampler*> Samplers;
        std::unordered_map<std::string, Texture*> Textures;
        std::multiset<model_part_t> Parts;
        vertex_layout_t VertexLayout;
        mesh_data_t* MeshData;
        gltfMaterial Material;
    private:
        fx::gltf::Document* document;
        bool binaryFile{ false };
        void loadFromFile();
        void parseDocument();
    };
    
} // vpsk

#endif //!VPSK_GLTF_MODEL_HPP
