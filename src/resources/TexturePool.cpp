#include "resources/TexturePool.hpp"
#include "command/TransferPool.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
namespace vpsk {

    TexturePool::TexturePool(const vpr::Device * dvc, const vpr::TransferPool * transfer_pool) : device(dvc), transferPool(transfer_pool) {}

    void TexturePool::AddMaterials(const std::vector<tinyobj::material_t>& materials, const std::string& path_prefix) {
        // Go through the given material and all the texture data to the texture data map.
        for (const auto& mtl : materials) {
            auto load_texture_data = [&](const std::string& tex) {
                if (tex.empty()) {
                    return;
                }
                else {
                    auto inserted = stbTextures.try_emplace(tex, std::make_unique<vpr::Texture<vpr::texture_2d_t>>(device));
                    inserted.first->second->CreateFromFile(std::string(path_prefix + tex).c_str());
                    if (inserted.second) {
                        // Added a texture, add to material textures.
                        materialTextures.emplace(mtl.name, inserted.first);
                    }
                }
            };

            auto load_texture_buffer_data = [&]() {
                auto inserted = materialUboData.try_emplace(mtl.name, material_ubo_data_t{
                    glm::vec4{ mtl.ambient[0], mtl.ambient[1], mtl.ambient[2], 0.0f },
                    glm::vec4{ mtl.diffuse[0], mtl.diffuse[1], mtl.diffuse[2], 0.0f },
                    glm::vec4{ mtl.specular[0], mtl.specular[1], mtl.specular[2], 0.0f },
                    glm::vec4{ mtl.transmittance[0], mtl.transmittance[1], mtl.transmittance[2], 0.0f },
                    glm::vec4{ mtl.emission[0], mtl.emission[1], mtl.emission[2], 0.0f },
                    misc_material_data_t{ mtl.shininess, mtl.ior, mtl.dissolve, mtl.illum },
                    pbr_material_data_t{ mtl.roughness, mtl.metallic, mtl.sheen, mtl.clearcoat_thickness,
                        mtl.clearcoat_roughness, mtl.anisotropy, mtl.anisotropy_rotation, 0.0f }
                });

                if (inserted.second) {
                    auto inserted = materialBuffers.emplace(mtl.name, std::make_unique<vpr::Buffer>(device));
                    auto& iter = inserted.first;
                    iter->second->CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, sizeof(material_ubo_data_t));
                    iter->second->CopyToMapped(&materialUboData.at(mtl.name), sizeof(material_ubo_data_t), 0);
                }
            }; 

            load_texture_data(mtl.ambient_texname);
            load_texture_data(mtl.diffuse_texname);
            load_texture_data(mtl.specular_texname);
            load_texture_data(mtl.specular_highlight_texname);
            load_texture_data(mtl.bump_texname);
            load_texture_data(mtl.displacement_texname);
            load_texture_data(mtl.alpha_texname);
            load_texture_data(mtl.reflection_texname);
            load_texture_data(mtl.roughness_texname);
            load_texture_data(mtl.metallic_texname);
            load_texture_data(mtl.sheen_texname);
            load_texture_data(mtl.normal_texname);
            load_texture_buffer_data();

            auto bounds = materialTextures.equal_range(mtl.name);
            auto& cmd = transferPool->Begin();
            for (auto iter = bounds.first; iter != bounds.second; ++iter) {
                iter->second->second->TransferToDevice(cmd);
            }
            transferPool->Submit();

        }
    }

}