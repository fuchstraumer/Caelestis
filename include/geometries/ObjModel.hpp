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

    struct DrawInfo {
        DrawInfo() {}
        DrawInfo(const VkCommandBuffer cmd, const VkPipelineLayout layout, const bool opaque_only, const bool textured, const size_t num_sets) : 
            Cmd(cmd), Layout(layout), RenderOnlyOpaque(opaque_only), UseTextures(textured), NumSetsBound(num_sets) {}
        bool RenderOnlyOpaque = false;
        bool UseTextures = true;
        const VkCommandBuffer Cmd = VK_NULL_HANDLE;
        const VkPipelineLayout Layout = VK_NULL_HANDLE;
        size_t NumSetsBound = 0;
    };

    class ObjModel : public TriangleMesh {
        ObjModel(const ObjModel&) = delete;
        ObjModel& operator=(const ObjModel&) = delete;
    public:

        ObjModel(const vpr::Device* dvc, TexturePool* resource_pool);
        ~ObjModel();

        void Render(const DrawInfo info);
        
        void LoadModelFromFile(const std::string& obj_model_filename, vpr::TransferPool* transfer_pool);

        const VkDescriptorSetLayout GetMaterialSetLayout() const noexcept;
        const AABB& GetAABB() const noexcept;

    private:

        void drawGeometry(const DrawInfo& info);
        void renderIdx(const VkCommandBuffer cmd, const VkPipelineLayout layout, const size_t idx);
        
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
        std::set<modelPart> parts;
        std::multimap<size_t, VkDrawIndexedIndirectCommand> indirectCommands;
        size_t numMaterials;
        uint32_t commandOffset = 0;

        void loadMeshes(const std::string& file, vpr::TransferPool* transfer_pool);
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