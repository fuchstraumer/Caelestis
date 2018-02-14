#include "geometries/ObjModel.hpp"
#include "resources/TexturePool.hpp"
#include "command/TransferPool.hpp"
#include "util/easylogging++.h"

using namespace vpr;

namespace vpsk {


    ObjModel::~ObjModel() {
    }

    void ObjModel::Render(const VkCommandBuffer& cmd) {
        bindBuffers(cmd);
        for (auto& part : parts) {
            vkCmdDrawIndexed(cmd, part.idxCount, 1, part.startIdx, part.vertexOffset, 0);
        }
    }

    void ObjModel::LoadModelFromFile(const std::string& obj_model_filename, TransferPool* transfer_pool) {
        
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string tinyobj_err;

        if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &tinyobj_err, obj_model_filename.c_str())) {
            LOG(ERROR) << "TinyObjLoader failed to load model file " << obj_model_filename << " , exiting.";
            LOG(ERROR) << "Load failed with error: " << tinyobj_err;
            throw std::runtime_error(tinyobj_err.c_str());
        }

        modelName = shapes.front().name;

        texturePool->AddMaterials(materials);

        loadMeshes(shapes, attrib, transfer_pool);
        
    }

    void ObjModel::loadMeshes(const std::vector<tinyobj::shape_t>& shapes, const tinyobj::attrib_t& attrib, TransferPool* transfer_pool) {
       
        std::unordered_map<vertex_t, uint32_t> unique_vertices{};
        
        for (const auto& shape : shapes) {
            modelPart part;
            part.startIdx = indices.size();
            part.vertexOffset = 0;
            for (const auto& idx : shape.mesh.indices) {

                ReserveIndices(NumIndices() + shape.mesh.indices.size());
                vertex_t vert;
                vert.pos = { attrib.vertices[3 * idx.vertex_index], attrib.vertices[3 * idx.vertex_index + 1], attrib.vertices[3 * idx.vertex_index + 2] };
                vert.normal = { attrib.normals[3 * idx.normal_index], attrib.normals[3 * idx.normal_index + 1], attrib.normals[3 * idx.normal_index + 2] };
                vert.uv = { attrib.texcoords[2 * idx.texcoord_index], 1.0f - attrib.texcoords[2 * idx.texcoord_index + 1] };
                aabb.Include(vert.pos);
                if (unique_vertices.count(vert) == 0) {
                    unique_vertices[vert] = AddVertex(vert);
                }

                AddIndex(unique_vertices[vert]);
            }

            part.idxCount = indices.size();
            parts.push_back(part);
        }

        UpdatePosition(aabb.Center());

        CreateBuffers(device);
        LOG(INFO) << "Loaded mesh data from .obj file, uploading to device now...";
        auto& cmd = transfer_pool->Begin();
        RecordTransferCommands(cmd);
        
        transfer_pool->Submit();
        LOG(INFO) << "Mesh data upload complete.";

    }

}