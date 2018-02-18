#include "resources/TexturePool.hpp"
#include "command/TransferPool.hpp"
#include "resource/DescriptorSet.hpp"
#include "resource/DescriptorPool.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
namespace vpsk {

    TexturePool::TexturePool(const vpr::Device * dvc, const vpr::TransferPool * transfer_pool) : device(dvc), transferPool(transfer_pool) {}

    void TexturePool::AddMaterials(const std::vector<tinyobj::material_t>& materials, const std::string& path_prefix) {
        // Go through the given material and all the texture data to the texture data maps
        for (const auto& mtl : materials) {

            auto cmd = transferPool->Begin();
            idxNameMap.emplace(idxNameMap.size(), mtl.name);

            auto load_texture_data = [&](const std::string& tex)->decltype(stbTextures)::const_iterator {
                if (tex.empty()) {
                    return stbTextures.cend();
                }
                else if (stbTextures.count(tex)) {
                    return stbTextures.find(tex);
                }
                else {
                    auto inserted = stbTextures.try_emplace(tex, std::make_unique<vpr::Texture<vpr::texture_2d_t>>(device));
                    inserted.first->second->CreateFromFile(std::string(path_prefix + tex).c_str());
                    inserted.first->second->TransferToDevice(cmd);
                    return inserted.first;
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

            materialTextures.emplace(mtl.name, material_texture_data_t());

            auto ambient_iter = load_texture_data(mtl.ambient_texname);
            if (ambient_iter != stbTextures.cend()) {
                materialTextures.at(mtl.name).Ambient = ambient_iter;
            }
            
            auto diffuse_iter = load_texture_data(mtl.diffuse_texname);
            if (diffuse_iter != stbTextures.cend()) {
                materialTextures.at(mtl.name).Diffuse = diffuse_iter;
            }

            auto specular_iter = load_texture_data(mtl.specular_texname);
            if (specular_iter != stbTextures.cend()) {
                materialTextures.at(mtl.name).Specular = specular_iter;
            }

            auto highlight_iter = load_texture_data(mtl.specular_highlight_texname);
            if (highlight_iter != stbTextures.cend()) {
                materialTextures.at(mtl.name).SpecularHighlight = highlight_iter;
            }

            auto bump_iter = load_texture_data(mtl.bump_texname);
            if (bump_iter != stbTextures.cend()) {
                materialTextures.at(mtl.name).Bump = bump_iter;
            }

            auto displ_iter = load_texture_data(mtl.displacement_texname);
            if (displ_iter != stbTextures.cend()) {
                materialTextures.at(mtl.name).Displacement = displ_iter;
            }

            auto alpha_iter = load_texture_data(mtl.alpha_texname);
            if (alpha_iter != stbTextures.cend()) {
                materialTextures.at(mtl.name).Alpha = alpha_iter;
            }

            auto refl_iter = load_texture_data(mtl.reflection_texname);
            if (refl_iter != stbTextures.cend()) {
                materialTextures.at(mtl.name).Reflection = refl_iter;
            }

            auto rough_iter = load_texture_data(mtl.roughness_texname);
            if (rough_iter != stbTextures.cend()) {
                materialTextures.at(mtl.name).Roughness = rough_iter;
            }

            auto metallic_iter = load_texture_data(mtl.metallic_texname);
            if (metallic_iter != stbTextures.cend()) {
                materialTextures.at(mtl.name).Roughness = rough_iter;
            }

            auto sheen_iter = load_texture_data(mtl.sheen_texname);
            if (sheen_iter != stbTextures.cend()) {
                materialTextures.at(mtl.name).Sheen = sheen_iter;
            }

            auto normal_iter = load_texture_data(mtl.normal_texname);
            if (normal_iter != stbTextures.cend()) {
                materialTextures.at(mtl.name).Normal = normal_iter;
            }

            load_texture_buffer_data();
            // We've been recording all our commands - now submit.
            transferPool->Submit();

        }

        createDescriptorPool();
    }

    void TexturePool::BindMaterialAtIdx(const size_t & idx, const VkCommandBuffer cmd, const VkPipelineLayout layout) {
        const auto& name = idxNameMap.at(idx);
        const auto& set = materialSets.at(name);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &set->vkHandle(), 0, nullptr);
    }

    void TexturePool::createDescriptorPool() {
        descriptorPool = std::make_unique<vpr::DescriptorPool>(device, idxNameMap.size());
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, idxNameMap.size());
        descriptorPool->Create();
    }

    void TexturePool::createDescriptorSets() {

        for (auto& entry : idxNameMap) {
            const auto& name = entry.second;
            materialSets.emplace(name, std::make_unique<vpr::DescriptorSet>(device));
            const auto& idx = entry.first;
            
            
        }
    }

}