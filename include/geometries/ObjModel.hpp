#pragma once
#ifndef VULPESRENDER_OBJ_MODEL_HPP
#define VULPESRENDER_OBJ_MODEL_HPP
#include "ForwardDecl.hpp"
#include "TriangleMesh.hpp"
#include "resource/Buffer.hpp"
#include "resources/Material.hpp"
#include "util/AABB.hpp"
#include <map>
#include <set>
namespace vpsk {

    class TexturePool;

    class ObjModel : public TriangleMesh {
        ObjModel(const ObjModel&) = delete;
        ObjModel& operator=(const ObjModel&) = delete;
    public:

        ObjModel(const vpr::Device* dvc, TexturePool* resource_pool);
        ~ObjModel();

        void Render(const VkCommandBuffer& cmd, const VkPipelineLayout layout);
        void LoadModelFromFile(const std::string& obj_model_filename, vpr::TransferPool* transfer_pool);

        const AABB& GetAABB() const noexcept;

    private:

        struct modelPart {
            uint32_t startIdx;
            uint32_t idxCount;
            int32_t vertexOffset;
            uint32_t mtlIdx;
            bool operator==(const modelPart& other) const noexcept {
                return (idxCount == other.idxCount) && (mtlIdx == other.mtlIdx);
            }
            bool operator<(const modelPart& other) const noexcept {
                if (mtlIdx == other.mtlIdx) {
                    return idxCount < other.idxCount;
                }
                else {
                    return mtlIdx < other.mtlIdx;
                }
            }
        };

        std::unique_ptr<vpr::Buffer> indirectDrawBuffer;
        std::multiset<modelPart> parts;
        std::multimap<size_t, VkDrawIndexedIndirectCommand> indirectCommands;
        size_t numMaterials;

        void loadMeshes(const std::vector<tinyobj::shape_t>& shapes, const tinyobj::attrib_t& attrib, vpr::TransferPool* transfer_pool);
        void generateIndirectDraws();
        void createIndirectDrawBuffer();
        TexturePool* texturePool;
        const vpr::Device* device;
        std::string modelName;
        AABB aabb;
        const bool multiDrawIndirect;
    };

}


#endif //!VULPESRENDER_OBJ_MODEL_HPP