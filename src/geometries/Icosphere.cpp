#include "vpr_stdafx.h"
#include "geometries/Icosphere.hpp"
#include "core/LogicalDevice.hpp"
#include "command/TransferPool.hpp"
#include "core/PhysicalDevice.hpp"
#include "scene/BaseScene.hpp"
#include <unordered_map>
#include "util/easylogging++.h"

using namespace vpr;

namespace vpsk {

    constexpr float golden_ratio = 1.61803398875f;
    constexpr float FLOAT_PI = 3.14159265359f;
    
    static const std::array<vertex_t, 12> initial_vertices {
        vertex_t{ glm::vec3(-golden_ratio, 1.0f, 0.0f) },
        vertex_t{ glm::vec3( golden_ratio, 1.0f, 0.0f) },
        vertex_t{ glm::vec3(-golden_ratio,-1.0f, 0.0f) },
        vertex_t{ glm::vec3( golden_ratio,-1.0f, 0.0f) },
        
        vertex_t{ glm::vec3( 0.0f,-golden_ratio, 1.0f) },
        vertex_t{ glm::vec3( 0.0f, golden_ratio, 1.0f) },
        vertex_t{ glm::vec3( 0.0f,-golden_ratio,-1.0f) },
        vertex_t{ glm::vec3( 0.0f, golden_ratio,-1.0f) },

        vertex_t{ glm::vec3( 1.0f, 0.0f,-golden_ratio) },
        vertex_t{ glm::vec3( 1.0f, 0.0f, golden_ratio) },
        vertex_t{ glm::vec3(-1.0f, 0.0f,-golden_ratio) },
        vertex_t{ glm::vec3(-1.0f, 0.0f, golden_ratio) }
    };

    constexpr static std::array<uint32_t, 60> initial_indices {
        0,11, 5, 
        0, 5, 1, 
        0, 1, 7, 
        0, 7,10, 
        0,10,11,

        5,11, 4, 
        1, 5, 9, 
        7, 1, 8,
       10, 7, 6,
       11,10, 2,

        3, 9, 4, 
        3, 4, 2, 
        3, 2, 6, 
        3, 6, 8, 
        3, 8, 9,

        4, 9, 5, 
        2, 4,11, 
        6, 2,10, 
        8, 6, 7, 
        9, 8, 1 
    };

    Icosphere::Icosphere(const vpr::Device* _dvc, const size_t& subdivision_level, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rotation) : TriangleMesh(pos, scale, rotation), subdivisionLevel(subdivision_level) {
        device = _dvc;
        updateModelMatrix();
    }

    Icosphere::~Icosphere() {
        DestroyVulkanObjects();
        FreeCpuData();
        descriptorSet.reset();
        cmpTexture.reset();
        texture.reset();
    }

    void Icosphere::SetTexture(const char* filename) {
        texturePath = std::string(filename);
    }

    void Icosphere::SetTexture(const char* filename, const VkFormat& compressed_texture_format) {
        texturePath = std::string(filename);
        textureFormat = compressed_texture_format;
    }

    void Icosphere::Init(const glm::mat4& projection, TransferPool* transfer_pool, DescriptorPool* descriptor_pool, vpr::DescriptorSetLayout* set_layout) {

        uboData.projection = projection;
        
        createMesh(subdivisionLevel);
        createTexture();
        uploadData(transfer_pool);
        createDescriptorSet(descriptor_pool, set_layout);
        createPipelineLayout();

    }

    void Icosphere::Render(const VkCommandBuffer& cmd_buffer, const VkPipelineLayout& layout) const {
        vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptorSet->vkHandle(), 0, nullptr);
        const glm::mat4 mvp = uboData.projection * uboData.view * uboData.model;
        vkCmdPushConstants(cmd_buffer, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), glm::value_ptr(mvp));
        vkCmdPushConstants(cmd_buffer, layout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::mat4), sizeof(glm::vec4) * 3, &uboData.lightPosition); 
        TriangleMesh::Render(cmd_buffer);
    }

    void Icosphere::CreateShaders(const std::string & vertex_shader_path, const std::string & fragment_shader_path) {
        vertPath = vertex_shader_path;
        fragPath = fragment_shader_path;
    }

    static inline glm::vec3 midpoint(const vertex_t& v0, const vertex_t v1) noexcept {
        return (v0.pos + v1.pos) / 2.0f;
    }

    void Icosphere::UpdateUBO(const glm::mat4 & view, const glm::vec3& viewer_position) noexcept {
        uboData.view = view;
        uboData.viewerPosition = glm::vec4(viewer_position.x, viewer_position.y, viewer_position.z, 1.0f);
    }

    void Icosphere::UpdateLightPosition(const glm::vec3& new_light_pos) noexcept {
        uboData.lightPosition = glm::vec4(new_light_pos.x, new_light_pos.y, new_light_pos.z, 1.0f);
    }

    void Icosphere::SetModelMatrix(const glm::mat4 & model) {
        uboData.model = model;
    }

    void Icosphere::createMesh(const size_t& subdivision_level) {
        subdivide(subdivision_level);
        calculateUVs();
        uboData.model = GetModelMatrix();
        CreateBuffers(device);
    }

    void Icosphere::subdivide(const size_t& subdivision_level) {

        indices.assign(initial_indices.cbegin(), initial_indices.cend());
        for(const auto& vertex : initial_vertices) {
            AddVertex(vertex);
        }

        for (int j = 0; j < subdivision_level; ++j) {
            const size_t num_triangles = NumIndices() / 3;
            for (uint32_t i = 0; i < num_triangles; ++i) {
                uint32_t i0 = indices[i * 3 + 0];
                uint32_t i1 = indices[i * 3 + 1];
                uint32_t i2 = indices[i * 3 + 2];

                uint32_t i3 = static_cast<uint32_t>(vertexPositions.size());
                uint32_t i4 = i3 + 1;
                uint32_t i5 = i4 + 1;

                indices[i * 3 + 1] = i3;
                indices[i * 3 + 2] = i5;

                indices.insert(indices.cend(), { i3, i1, i4, i5, i3, i4, i5, i4, i2 });

                const glm::vec3 midpoint0 = 0.5f * (vertexPositions[i0] + vertexPositions[i1]);
                const glm::vec3 midpoint1 = 0.5f * (vertexPositions[i1] + vertexPositions[i2]);
                const glm::vec3 midpoint2 = 0.5f * (vertexPositions[i2] + vertexPositions[i0]);

                AddVertex(vertex_t{ midpoint0, midpoint0 });
                AddVertex(vertex_t{ midpoint1, midpoint1 });
                AddVertex(vertex_t{ midpoint2, midpoint2 });
            }
        }
        
        for(size_t i = 0; i < vertexPositions.size(); ++i) {
            vertexPositions[i] = glm::normalize(vertexPositions[i]);
            vertexData[i].normal = vertexPositions[i];
        }

    }

    void Icosphere::calculateUVs() {
        for(size_t i = 0; i < vertexPositions.size(); ++i) {
            const glm::vec3& norm = vertexData[i].normal;
            vertexData[i].uv.x = (glm::atan(norm.x, -norm.z) / FLOAT_PI) * 0.5f + 0.5f;
            vertexData[i].uv.y = -norm.y * 0.5f + 0.5f;
        }

        auto add_vertex_w_uv = [&](const size_t& i, const glm::vec2& uv) {
            const uint32_t& idx = indices[i];
            indices[i] =  AddVertex(vertex_t{ vertexPositions[idx], vertexData[idx].normal, glm::vec3(), uv });
        };

        const size_t num_triangles = indices.size() / 3;
        for(size_t i = 0; i < num_triangles; ++i) {
            const glm::vec2& uv0 = vertexData[indices[i * 3]].uv;
            const glm::vec2& uv1 = vertexData[indices[i * 3 + 1]].uv;
            const glm::vec2& uv2 = vertexData[indices[i * 3 + 2]].uv;
            const float d1 = uv1.x - uv0.x;
            const float d2 = uv2.x - uv0.x;
            if(std::abs(d1) > 0.5f && std::abs(d2) > 0.5f){
                add_vertex_w_uv(i * 3, uv0 + glm::vec2((d1 > 0.0f) ? 1.0f : -1.0f, 0.0f));
            }
            else if(std::abs(d1) > 0.5f) {
                add_vertex_w_uv(i * 3 + 1, uv1 + glm::vec2((d1 < 0.0f) ? 1.0f : -1.0f, 0.0f));
            }
            else if(std::abs(d2) > 0.5f) {
                add_vertex_w_uv(i * 3 + 2, uv2 + glm::vec2((d2 < 0.0f) ? 1.0f : -1.0f, 0.0f));
            }
        }
    }

    void Icosphere::uploadData(TransferPool* transfer_pool) {

        auto& cmd = transfer_pool->Begin();
        RecordTransferCommands(cmd);
        if(textureFormat == VK_FORMAT_R8G8B8A8_UNORM) {
            texture->TransferToDevice(cmd);
        }
        else {
            cmpTexture->TransferToDevice(cmd);
        }
        transfer_pool->Submit();

    }

    void Icosphere::createDescriptorSet(DescriptorPool* descriptor_pool, vpr::DescriptorSetLayout* set_layout) {
        descriptorSet = std::make_unique<DescriptorSet>(device);
        if (textureFormat == VK_FORMAT_R8G8B8A8_UNORM) {
            descriptorSet->AddDescriptorInfo(texture->GetDescriptor(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0);
        }
        else {
            descriptorSet->AddDescriptorInfo(cmpTexture->GetDescriptor(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0);
        }
        descriptorSet->Init(descriptor_pool, set_layout);
    }

    void Icosphere::createTexture() {

        if(textureFormat == VK_FORMAT_R8G8B8A8_UNORM) {
            texture = std::make_unique<Texture<texture_2d_t>>(device);
            texture->CreateFromFile(texturePath.c_str());
            VkSamplerCreateInfo sampler_info = vk_sampler_create_info_base;
            sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            texture->CreateSampler(sampler_info);
        } 
        else {
            cmpTexture = std::make_unique<Texture<gli::texture2d>>(device);
            cmpTexture->CreateFromFile(texturePath.c_str(), textureFormat);
            VkSamplerCreateInfo sampler_info = vk_sampler_create_info_base;
            sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler_info.anisotropyEnable = VK_TRUE;
            sampler_info.maxAnisotropy = VK_SAMPLE_COUNT_8_BIT;
            cmpTexture->CreateSampler(sampler_info);
        }

    }

}