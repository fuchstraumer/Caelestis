#pragma once
#ifndef VULPESRENDER_ICOSPHERE_HPP
#define VULPESRENDER_ICOSPHERE_HPP

#include "TriangleMesh.hpp"
#include "resource/Texture.hpp"
#include "resource/Buffer.hpp"
#include "resource/ShaderModule.hpp"
#include "resource/DescriptorSet.hpp"
#include "resource/PipelineLayout.hpp"
#include "resource/PipelineCache.hpp"
#include "render/GraphicsPipeline.hpp"
#include "resource/Texture.hpp"
#include "resource/DescriptorSet.hpp"
#include "vertex_t.hpp"

namespace vpsk {

    /** Defines an icosphere object that is subdivided from the base 12 vertices based on detail_level. Each additional level will double the triangle count,
    *   but an attempt to use shared vertices is made and should reduce or avoid duplicated vertices. Unlike Billboard and Skybox, you must specify shaders to 
    *   use on this boject as no defaults exist (and no suitable default came to mind).
    *
    *   UVs are dynamically generated after the mesh has been created, and normals are created per-vertex by normalizing the initial vertex positions (which 
    *   should be positions on the unit sphere). This object can use compressed or uncompressed texture formats, so long as the format of the compressed texture
    *   is given when calling the SetTexture method.
    *
    *   \ingroup Objects
    */
    class Icosphere : public TriangleMesh {
        Icosphere(const Icosphere&) = delete;
        Icosphere& operator=(const Icosphere&) = delete;
    public:

        Icosphere(const size_t& detail_level, const glm::vec3& position, const glm::vec3& scale = glm::vec3(1.0f), const glm::vec3& rotation = glm::vec3(0.0f));
        ~Icosphere();

        void Init(const vpr::Device* dvc, const glm::mat4& projection, const VkRenderPass& renderpass, vpr::TransferPool* transfer_pool, vpr::DescriptorPool* descriptor_pool);
        void Render(const VkCommandBuffer& cmd_buffer, const VkCommandBufferBeginInfo& begin_info, const VkViewport& viewport, const VkRect2D& scissor);
        void CreateShaders(const std::string& vertex_shader_path, const std::string& fragment_shader_path);
        void SetTexture(const char* filename);
        void SetTexture(const char* filename, const VkFormat& compressed_texture_format);
        void createShadersImpl();
        void UpdateUBO(const glm::mat4& view, const glm::vec3& viewer_position) noexcept;
        void UpdateLightPosition(const glm::vec3 & new_light_pos) noexcept;
        void SetModelMatrix(const glm::mat4& model) override;
        
    private:


        void createMesh(const size_t& subdivision_level);
        void uploadData(vpr::TransferPool* transfer_pool);
        void createPipelineCache();
        void createPipelineLayout();
        void setPipelineStateInfo();
        void createGraphicsPipeline(const VkRenderPass& render_pass);
        void createTexture();
        void createDescriptorSet(vpr::DescriptorPool* descriptor_pool);
        void subdivide(const size_t& subdivision_level);
        void calculateUVs();

        struct ubo_data_t {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 projection;
            glm::vec4 lightPosition = glm::vec4(0.0f, 300.0f, 0.0f, 1.0f);
            glm::vec4 viewerPosition = glm::vec4(0.0f);
            glm::vec4 lightColor = glm::vec4(1.0f);
        } uboData;

        constexpr static VkVertexInputBindingDescription bindingDescription{ 0, sizeof(vertex_t), VK_VERTEX_INPUT_RATE_VERTEX };
        constexpr static VkVertexInputAttributeDescription attributeDescription{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 };

        std::unique_ptr<vpr::ShaderModule> vert, frag;
        std::unique_ptr<vpr::DescriptorSet> descriptorSet;
        std::unique_ptr<vpr::PipelineCache> pipelineCache;
        std::unique_ptr<vpr::PipelineLayout> pipelineLayout;
        std::unique_ptr<vpr::GraphicsPipeline> graphicsPipeline;
        std::unique_ptr<vpr::Texture<vpr::texture_2d_t>> texture;

        vpr::GraphicsPipelineInfo pipelineStateInfo;
        VkGraphicsPipelineCreateInfo pipelineCreateInfo;
        size_t subdivisionLevel;

        VkFormat textureFormat = VK_FORMAT_R8G8B8A8_UNORM;
        std::string vertPath, fragPath, texturePath;

    };


}

#endif //!VULPESRENDER_ICOSPHERE_HPP