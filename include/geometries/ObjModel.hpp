#pragma once
#ifndef VULPESRENDER_OBJ_MODEL_HPP
#define VULPESRENDER_OBJ_MODEL_HPP
#include "ForwardDecl.hpp"
#include "TriangleMesh.hpp"
#include "resource/Buffer.hpp"
#include "resources/Material.hpp"
#include "util/AABB.hpp"

namespace vpsk {

    class TexturePool;

    class ObjModel : public TriangleMesh {
        ObjModel(const ObjModel&) = delete;
        ObjModel& operator=(const ObjModel&) = delete;
    public:

        ObjModel(const vpr::Device* dvc, TexturePool* resource_pool);
        ~ObjModel();

        void Render(const VkCommandBuffer& cmd);
        void LoadModelFromFile(const std::string& obj_model_filename, vpr::TransferPool* transfer_pool);

        const AABB& GetAABB() const noexcept;

    private:
        static std::map<std::string, std::unique_ptr<Material>> materialsMap;
        struct modelPart {
            std::map<std::string, std::unique_ptr<Material>>::iterator material;
            uint32_t startIdx;
            uint32_t idxCount;
            int32_t vertexOffset;
        };
        std::vector<modelPart> parts;

        void loadMeshes(const std::vector<tinyobj::shape_t>& shapes, const tinyobj::attrib_t& attrib, vpr::TransferPool* transfer_pool);

        TexturePool* texturePool;
        vpr::DescriptorPool* descriptorPool;
        const vpr::Device* device;
        std::string modelName;
        AABB aabb;
        
    };

}


#endif //!VULPESRENDER_OBJ_MODEL_HPP