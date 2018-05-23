#include "geometries/ObjModel.hpp"
#include "resources/TexturePool.hpp"
#include "command/TransferPool.hpp"
#include "util/easylogging++.h"
#include "core/LogicalDevice.hpp"
#include "core/PhysicalDevice.hpp"
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

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string tinyobj_err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &tinyobj_err, obj_model_filename.c_str(), "../rsrc/crytekSponza/")) {
            LOG(ERROR) << "TinyObjLoader failed to load model file " << obj_model_filename << " , exiting.";
            LOG(ERROR) << "Load failed with error: " << tinyobj_err;
            throw std::runtime_error(tinyobj_err.c_str());
        }

        modelName = shapes.front().name;
        texturePool->AddMaterials(materials, "../rsrc/crytekSponza/");
        numMaterials = materials.size();
        loadMeshes(shapes, attrib, transfer_pool);
        generateIndirectDraws();
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

    void ObjModel::loadMeshes(const std::vector<tinyobj::shape_t>& shapes, const tinyobj::attrib_t& attrib, TransferPool* transfer_pool) {

        size_t num_vertices_loaded = 0;

        std::vector<material_group_t> groups(numMaterials + 1);
        std::vector<std::unordered_map<vertex_t, uint32_t>> uniqueVertices(numMaterials + 1);

        auto appendVert = [&groups, &uniqueVertices](const vertex_t& vert, const int material_id, AABB& bounds) {
            auto& unique_verts = uniqueVertices[material_id + 1];
            auto& group = groups[material_id + 1];
            group.positions.push_back(vert.pos);
            group.data.push_back(TriangleMesh::vertex_data_t{ vert.normal, glm::vec3(0.0f), vert.uv });
            bounds.Include(vert.pos);
            group.indices.push_back(group.positions.size() - 1);
        };

        for (const auto& shape : shapes) {
            size_t idxOffset = 0;
            for (size_t i = 0; i < shape.mesh.num_face_vertices.size(); ++i) {
                auto ngon = shape.mesh.num_face_vertices[i];
                auto material_id = shape.mesh.material_ids[i];
                for (size_t j = 0; j < ngon; ++j) {
                    const auto& idx = shape.mesh.indices[idxOffset + j];

                    vertex_t vert;
                    vert.pos = glm::vec3{
                        attrib.vertices[3 * idx.vertex_index + 0],
                        attrib.vertices[3 * idx.vertex_index + 1],
                        attrib.vertices[3 * idx.vertex_index + 2]
                    };
                    vert.normal = glm::vec3{
                        attrib.normals[3 * idx.normal_index + 0],
                        attrib.normals[3 * idx.normal_index + 1],
                        attrib.normals[3 * idx.normal_index + 2]
                    };
                    vert.uv = glm::vec2{
                        attrib.texcoords[2 * idx.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]
                    };

                    appendVert(vert, material_id, aabb);
                }

                idxOffset += ngon;
            }
        }

        int mtl_idx = -1;
        for (auto& group : groups) {
            modelPart new_part;
            new_part.vertexOffset = vertexPositions.size();
            vertexPositions.insert(vertexPositions.end(), group.positions.begin(), group.positions.end());
            vertexData.insert(vertexData.end(), group.data.begin(), group.data.end());
            new_part.startIdx = indices.size();
            indices.insert(indices.end(), group.indices.begin(), group.indices.end());
            new_part.idxCount = group.indices.size();
            new_part.mtlIdx = mtl_idx;
            ++mtl_idx;
            parts.insert(new_part);
        }

        CreateBuffers(device);
        LOG(INFO) << "Loaded mesh data from .obj file, uploading to device now...";
        auto& cmd = transfer_pool->Begin();
        RecordTransferCommands(cmd);
        transfer_pool->Submit();
        LOG(INFO) << "Mesh data upload complete.";

    }

    void ObjModel::generateIndirectDraws() {
        for (auto& idx_group : parts) {
            if (idx_group.mtlIdx > numMaterials + 1) {
                continue;
            }
            if (indirectCommands.count(idx_group.mtlIdx)) {
                auto range = indirectCommands.equal_range(idx_group.mtlIdx);
                auto same_idx_entry = std::find_if(range.first, range.second, [idx_group](const decltype(indirectCommands)::value_type& elem) { return elem.second.indexCount == idx_group.idxCount; });
                if (same_idx_entry == indirectCommands.end()) {
                    indirectCommands.emplace(idx_group.mtlIdx, VkDrawIndexedIndirectCommand{ idx_group.idxCount, 1, idx_group.startIdx, idx_group.vertexOffset, 0 });
                }
                else {
                    same_idx_entry->second.instanceCount++;
                }
            }
            else {
                indirectCommands.emplace(idx_group.mtlIdx, VkDrawIndexedIndirectCommand{ idx_group.idxCount, 1, idx_group.startIdx, idx_group.vertexOffset, 0 });
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