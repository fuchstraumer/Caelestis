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

    ObjModel::ObjModel(const vpr::Device * dvc, TexturePool * resource_pool) : device(dvc), texturePool(resource_pool), multiDrawIndirect(supports_multidraw_indirect(dvc)) {}

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

        if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &tinyobj_err, obj_model_filename.c_str(), "../rsrc/crytekSponza/")) {
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



    void ObjModel::loadMeshes(const std::vector<tinyobj::shape_t>& shapes, const tinyobj::attrib_t& attrib, TransferPool* transfer_pool) {
       
        size_t num_vertices_loaded = 0;

        auto sort_parts = [](const tinyobj::shape_t& p0, const tinyobj::shape_t& p1) {
            if (p0.mesh.material_ids.front() == p1.mesh.material_ids.front()) {
                return p0.mesh.indices.size() < p1.mesh.indices.size();
            }
            else {
                return p0.mesh.material_ids.front() < p1.mesh.material_ids.front();
            }
        };


        std::vector<tinyobj::shape_t> sorted_shapes{ shapes.begin(), shapes.end() };
        auto remove_iter = std::remove_if(sorted_shapes.begin(), sorted_shapes.end(), [](const tinyobj::shape_t& shape) {
            return std::any_of(shape.mesh.material_ids.cbegin(), shape.mesh.material_ids.cend(), [](const int& i) { return i == -1; });
        });
        sorted_shapes.erase(remove_iter, sorted_shapes.end());
        std::sort(sorted_shapes.begin(), sorted_shapes.end(), sort_parts);


        int32_t vtx_offset = 0;
        for (const auto& shape : sorted_shapes) {
            modelPart part;
            part.startIdx = static_cast<uint32_t>(indices.size());
            part.vertexOffset = vtx_offset;
            std::unordered_map<vertex_t, uint32_t> unique_vertices{};
            uint32_t idx_count = 0;
            uint32_t vtx_count = 0;
            for (const auto& idx : shape.mesh.indices) {
                vertex_t vert;
                vert.pos = { attrib.vertices[3 * idx.vertex_index], attrib.vertices[3 * idx.vertex_index + 1], attrib.vertices[3 * idx.vertex_index + 2] };
                vert.normal = { attrib.normals[3 * idx.normal_index], attrib.normals[3 * idx.normal_index + 1], attrib.normals[3 * idx.normal_index + 2] };
                vert.uv = { attrib.texcoords[2 * idx.texcoord_index], 1.0f - attrib.texcoords[2 * idx.texcoord_index + 1] };
                aabb.Include(vert.pos);
                if (unique_vertices.count(vert) == 0) {
                    unique_vertices[vert] = AddVertex(vert);
                    ++vtx_count;
                }

                AddIndex(unique_vertices[vert]);
                ++idx_count;
            }

            part.idxCount = idx_count;
            part.mtlIdx = shape.mesh.material_ids.front();
            vtx_offset += static_cast<int32_t>(unique_vertices.size());
            parts.insert(part);
        }
        UpdatePosition(aabb.Center());
        

        CreateBuffers(device);
        LOG(INFO) << "Loaded mesh data from .obj file, uploading to device now...";
        auto& cmd = transfer_pool->Begin();
        RecordTransferCommands(cmd);
        transfer_pool->Submit();
        LOG(INFO) << "Mesh data upload complete.";

    }

    void ObjModel::generateIndirectDraws() {
        for (auto& idx_group : parts) {
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