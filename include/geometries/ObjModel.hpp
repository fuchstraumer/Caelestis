#pragma once
#ifndef VULPESRENDER_OBJ_MODEL_HPP
#define VULPESRENDER_OBJ_MODEL_HPP

#include "TriangleMesh.hpp"
#include "resource/Texture.hpp"
#include "resource/Buffer.hpp"
#include "resource/ShaderModule.hpp"
#include "resource/DescriptorSet.hpp"
#include "resource/PipelineLayout.hpp"
#include "resource/PipelineCache.hpp"
#include "render/GraphicsPipeline.hpp"
#include "resources/Material.hpp"
#include "util/AABB.hpp"

namespace vpsk {


    class ObjModel : public TriangleMesh {
        ObjModel(const ObjModel&) = delete;
        ObjModel& operator=(const ObjModel&) = delete;
        
        struct obj_model_ubo_t {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 projection;
        };
    public:

        ObjModel(const Device* dvc);
        ~ObjModel();

        void Render(const VkCommandBuffer& cmd, const VkCommandBufferBeginInfo& begin, const VkViewport& view, const VkRect2D& scissor);
        void LoadModelFromFile(const std::string& obj_model_filename, vpr::TransferPool* transfer_pool);
        void CreateShaders(const std::string& vertex_shader_path, const std::string& fragment_shader_path);
        void CompleteSetup(const glm::mat4& projection, const VkRenderPass& renderpass);
        void UpdateUBO(const glm::mat4& view);

        const AABB& GetAABB() const noexcept;
    private:

        void loadMeshes(const std::vector<tinyobj::shape_t>& shapes, const tinyobj::attrib_t& attrib, vpr::TransferPool* transfer_pool);
        void createPipelineCache();
        void createPipelineLayout();
        void setPipelineStateInfo();
        void createGraphicsPipeline(const VkRenderPass& renderpass);

        vpr::DescriptorPool* descriptorPool;
        const vpr::Device* device;

        std::map<std::string, std::unique_ptr<Material>>::iterator myMaterial;

        std::unique_ptr<vpr::ShaderModule> vert, frag;
        std::unique_ptr<vpr::PipelineLayout> pipelineLayout;
        std::unique_ptr<vpr::PipelineCache> pipelineCache;
        std::unique_ptr<vpr::GraphicsPipeline> graphicsPipeline;
        vpr::GraphicsPipelineInfo pipelineStateInfo;
        VkGraphicsPipelineCreateInfo pipelineCreateInfo;
        std::string modelName;
        AABB aabb;
        
    };

}


#endif //!VULPESRENDER_OBJ_MODEL_HPP