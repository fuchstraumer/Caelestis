#pragma once
#ifndef VULPESRENDER_SKYBOX_OBJECT_HPP
#define VULPESRENDER_SKYBOX_OBJECT_HPP

#include "TriangleMesh.hpp"
#include "resources/Texture.hpp"
#include "resource/Buffer.hpp"
#include "resource/ShaderModule.hpp"
#include "resource/DescriptorSet.hpp"
#include "resource/PipelineLayout.hpp"
#include "resource/PipelineCache.hpp"
#include "render/GraphicsPipeline.hpp"

namespace vpsk {

    /** Does what it says on the tin and nothing more. The texture used must be of a format like DDS or KTX that supports having all size cubemap faces in
    *   one file. Not only does this allow for high-resolution and performant textures, but its required for how the texture loading system for cubemaps currently
    *   functions. Be sure to keep the UBO data updated: this is required to sustain the "skybox" illusion properly.
    *   So long as the resource path is set in the vulpesSceneConfig struct, suitable default shaders are included with this project and will be loaded. 
    *   \todo See if loading cubemaps or the six cube face images can be supported via STB to allow for uncompressed formats.
    *   \ingroup Objects
    */  
    class Skybox : public TriangleMesh {
        Skybox(const Skybox&) = delete;
        Skybox& operator=(const Skybox&) = delete;
    public:

        Skybox(const vpr::Device* dvc);
        ~Skybox();

        void CreateTexture(const std::string& texture_filename, const VkFormat& texture_format);
        void Create(const glm::mat4& projection, const VkRenderPass& render_pass, vpr::TransferPool* transfer_pool, vpr::DescriptorPool* descriptor_pool);
        void Render(const VkCommandBuffer& draw_cmd, const VkCommandBufferBeginInfo& begin_info, const VkViewport& viewport, const VkRect2D& scissor);
        void UpdateUBO(const glm::mat4& view_matrix) noexcept;

    private:

        void createMesh();
        void uploadData(vpr::TransferPool* transfer_pool);
        void createTexture(const std::string& filename, const VkFormat& file_format);
        void createShaders();
        void createPipelineCache();
        void createSetLayout();
        void setupDescriptorSet(vpr::DescriptorPool* descriptor_pool);
        void setupPipelineLayout();
        void setPipelineStateInfo();
        void createGraphicsPipeline(const VkRenderPass& render_pass);

        struct ubo_data_t {
            glm::mat4 view;
            glm::mat4 projection;
        } uboData;

        std::unique_ptr<vpr::Texture<gli::texture_cube>> texture;
        std::unique_ptr<vpr::ShaderModule> vert, frag;
        std::unique_ptr<vpr::DescriptorSet> descriptorSet;
        std::unique_ptr<vpr::PipelineCache> pipelineCache;
        std::unique_ptr<vpr::PipelineLayout> pipelineLayout;
        std::unique_ptr<vpr::GraphicsPipeline> graphicsPipeline;
        std::unique_ptr<vpr::DescriptorSetLayout> setLayout;
        vpr::GraphicsPipelineInfo graphicsPipelineStateInfo;
        VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo;

    };

}

#endif //!VULPESRENDER_SKYBOX_OBJECT_HPP