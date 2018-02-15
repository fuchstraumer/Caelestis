#pragma once
#ifndef VULPESRENDER_OBJ_MODEL_HPP
#define VULPESRENDER_OBJ_MODEL_HPP
#include "ForwardDecl.hpp"
#include "TriangleMesh.hpp"
#include "resource/Buffer.hpp"
#include "resources/Material.hpp"
#include "util/AABB.hpp"
#include <unordered_map>
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
        };
        std::unordered_multimap<size_t, modelPart> parts;
        size_t numMaterials;

        void loadMeshes(const std::vector<tinyobj::shape_t>& shapes, const tinyobj::attrib_t& attrib, vpr::TransferPool* transfer_pool);

        TexturePool* texturePool;
        const vpr::Device* device;
        std::string modelName;
        AABB aabb;
        
    };

}


#endif //!VULPESRENDER_OBJ_MODEL_HPP