#include "geometries/ObjModel.hpp"
#include "resources/TexturePool.hpp"
#include "command/TransferPool.hpp"
#include "util/easylogging++.h"
#include "core/LogicalDevice.hpp"
#include "core/PhysicalDevice.hpp"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/cimport.h"
using namespace vpr;

namespace vpsk {

    bool supports_multidraw_indirect(const vpr::Device* dvc) {
        return dvc->GetPhysicalDeviceProperties().limits.maxDrawIndirectCount != 1;
    }

    ObjModel::ObjModel(const vpr::Device * dvc, TexturePool * resource_pool) : TriangleMesh(glm::vec3(0.0f)), device(dvc), texturePool(resource_pool), multiDrawIndirect(supports_multidraw_indirect(dvc)) {}

    ObjModel::~ObjModel() {}

    void ObjModel::Render(const DrawInfo info) {
        commandOffset = 0;

        auto bind_for_no_textures = [&](const VkCommandBuffer cmd) {
            constexpr VkDeviceSize offset{ 0 };
            vkCmdBindVertexBuffers(cmd, 0, 1, &vbo0->vkHandle(), &offset);
            vkCmdBindIndexBuffer(cmd, ebo->vkHandle(), 0, VK_INDEX_TYPE_UINT32);
        };

        if (!info.UseTextures) {
            bind_for_no_textures(info.Cmd);
        }
        else {
            bindBuffers(info.Cmd);
        }

        drawGeometry(info);


    }

    void ObjModel::drawGeometry(const DrawInfo& info) {
        for (size_t i = 0; i < numMaterials; ++i) {
            const auto& ubo = texturePool->GetMaterialUBO(i);
            if (info.RenderOnlyOpaque ? ubo.data.alpha == 1.0f : ubo.data.alpha < 1.0f) {
                if (info.UseTextures) {
                    texturePool->BindMaterialAtIdx(i, info.Cmd, info.Layout, info.NumSetsBound);
                }
                renderIdx(info.Cmd, info.Layout, i);
            }
            else {
                auto draw_ranges = indirectCommands.equal_range(i);
                const auto num_commands = std::distance(draw_ranges.first, draw_ranges.second);
                commandOffset += static_cast<uint32_t>(sizeof(VkDrawIndexedIndirectCommand) * num_commands);
            }
        }
    }

    void ObjModel::renderIdx(const VkCommandBuffer cmd, const VkPipelineLayout layout, const size_t idx) {
        auto draw_ranges = indirectCommands.equal_range(idx);
        const uint32_t num_commands = static_cast<uint32_t>(std::distance(draw_ranges.first, draw_ranges.second));
        if (multiDrawIndirect) {
            vkCmdDrawIndexedIndirect(cmd, indirectDrawBuffer->vkHandle(), commandOffset, num_commands, sizeof(VkDrawIndexedIndirectCommand));
            commandOffset += static_cast<uint32_t>(sizeof(VkDrawIndexedIndirectCommand)) * num_commands;
        }
        else {
            for (size_t j = 0; j < num_commands; ++j) {
                vkCmdDrawIndexedIndirect(cmd, indirectDrawBuffer->vkHandle(), commandOffset, 1, sizeof(VkDrawIndexedIndirectCommand));
                commandOffset += static_cast<uint32_t>(sizeof(VkDrawIndexedIndirectCommand));
            }
        }
    }

    void ObjModel::LoadModelFromFile(const std::string& obj_model_filename, TransferPool* transfer_pool) {
        loadMeshes(obj_model_filename, transfer_pool);
        createIndirectDrawBuffer();
        
    }

    const VkDescriptorSetLayout ObjModel::GetMaterialSetLayout() const noexcept {
        return texturePool->GetSetLayout();
    }

    const AABB & ObjModel::GetAABB() const noexcept {
        return aabb;
    }

    struct material_group_t {
        std::vector<glm::vec3> positions;
        std::vector<TriangleMesh::vertex_data_t> data;
        std::vector<uint32_t> indices;
    };


    void ObjModel::loadMeshes(const std::string & file, vpr::TransferPool * transfer_pool) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(file, aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_PreTransformVertices |
            aiProcess_ImproveCacheLocality | aiProcess_RemoveRedundantMaterials | aiProcess_OptimizeMeshes | aiProcess_SplitLargeMeshes);
        if (!scene) {
            throw std::runtime_error("");
        }
        else {

            uint32_t vertexCount = 0;
            uint32_t indexCount = 0;

            for (size_t i = 0; i < scene->mNumMeshes; ++i) {
                const aiMesh* mesh = scene->mMeshes[i];
                modelPart part;
                part.startIdx = indexCount;
                part.vertexOffset = vertexCount;
                vertexCount += mesh->mNumVertices;
                
                for (size_t j = 0; j < mesh->mNumVertices; ++j) {
                    const aiVector3D* pos = &(mesh->mVertices[j]);
                    const aiVector3D* norm = &(mesh->mNormals[j]);
                    const aiVector3D* tangent = &(mesh->mTangents[j]);
                    const aiVector3D* bitangent = &(mesh->mBitangents[j]);
                    const aiVector3D* uv = &(mesh->mTextureCoords[0][j]);

                    vertex_t vert{
                        glm::vec3{ pos->x, pos->y, pos->z },
                        glm::vec3{ norm->x, norm->y, norm->z },
                        glm::vec3{ tangent->x, tangent->y, tangent->z },
                        glm::vec2{ uv->x, uv->y }
                    };

                    aabb.Include(vert.pos);
                    AddVertex(vert);
                }

                part.startIdx = static_cast<uint32_t>(indices.size());
                part.idxCount = 0;
                for (size_t j = 0; j < mesh->mNumFaces; ++j) {
                    const aiFace& face = mesh->mFaces[j];
                    if (face.mNumIndices != 3) {
                        continue;
                    }
                    AddIndex(part.startIdx + face.mIndices[0]);
                    AddIndex(part.startIdx + face.mIndices[1]);
                    AddIndex(part.startIdx + face.mIndices[2]);
                    part.idxCount += 3;
                }

                indexCount = static_cast<uint32_t>(indices.size());
                vertexCount = static_cast<uint32_t>(vertexPositions.size());
                parts.insert(part);
            }
        }
    }

    void ObjModel::createIndirectDrawBuffer() {
        indirectDrawBuffer = std::make_unique<vpr::Buffer>(device);
        indirectDrawBuffer->CreateBuffer(VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(VkDrawIndexedIndirectCommand) * indirectCommands.size());
        std::vector<VkDrawIndexedIndirectCommand> indirect_buffer; indirect_buffer.reserve(indirectCommands.size());
        for (auto& drawCmd : indirectCommands) {
            indirect_buffer.push_back(drawCmd.second);
        }
        indirectDrawBuffer->CopyToMapped(indirect_buffer.data(), indirectDrawBuffer->Size(), 0);
        LOG(INFO) << "Generated indexed indirect draw commands for an obj file. Initial part count: " << std::to_string(parts.size()) << " . Indirect draw count: " << std::to_string(indirect_buffer.size());
    }

}