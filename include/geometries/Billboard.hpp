#pragma once
#ifndef VULPESRENDER_BILLBOARD_OBJECT_HPP
#define VULPESRENDER_BILLBOARD_OBJECT_HPP
#include "ForwardDecl.hpp"
#include <memory>
#include <vulkan/vulkan.h>
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include "render/GraphicsPipeline.hpp"
#include "resource/Texture.hpp"

namespace vpsk {
    
    /** Defines a billboard/sprite object that will always face the camera. Texture used for the sprite can be of any format.
    *   \ingroup Objects
    */
    class Billboard {
        Billboard(const Billboard&) = delete;
        Billboard& operator=(const Billboard&) = delete;
    public:

        Billboard(const glm::vec3& pos, const glm::vec3& scale, const glm::mat4& projection);
        ~Billboard();

        void SetSurfaceTexture(const char* texture_filename);
        void Init(const vpr::Device* dvc, const VkRenderPass& renderpass, vpr::TransferPool* transfer_pool, vpr::DescriptorPool* descriptor_pool);
        void Render(const VkCommandBuffer& cmd_, const VkCommandBufferBeginInfo& begin_info_, const VkViewport& viewport, const VkRect2D& scissor);
        void UpdateUBO(const glm::mat4& view);

    protected:

        void createBuffers();
        void transferResources(vpr::TransferPool* transfer_pool_);
        void createShaders();
        void createDescriptorSet(vpr::DescriptorPool* descriptor_pool);
        void createPipelineLayout();
        void createPipelineCache();
        void setPipelineStateInfo();
        void createGraphicsPipeline(const VkRenderPass& renderpass);

        constexpr static VkVertexInputBindingDescription bindingDescription {
            0, sizeof(float) * 5, VK_VERTEX_INPUT_RATE_VERTEX
        };

        constexpr static  std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions {
            VkVertexInputAttributeDescription{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
            VkVertexInputAttributeDescription{ 1, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 3 }
        };

        glm::vec3 position, scale;
        const vpr::Device* device = nullptr;

        std::unique_ptr<vpr::Buffer> vbo;
        std::unique_ptr<vpr::DescriptorSet> descriptorSet;
        std::unique_ptr<vpr::DescriptorSetLayout> setLayout;
        std::unique_ptr<vpr::PipelineLayout> pipelineLayout;
        std::unique_ptr<vpr::PipelineCache> pipelineCache;
        std::unique_ptr<vpr::ShaderModule> vert, frag;
        std::unique_ptr<vpr::GraphicsPipeline> pipeline;
        std::unique_ptr<vpr::Texture<vpr::texture_2d_t>> texture;

        vpr::GraphicsPipelineInfo pipelineStateInfo;
        VkGraphicsPipelineCreateInfo pipelineCreateInfo;

        struct billboard_ubo_data_t {
            glm::mat4 view;
            glm::mat4 projection;
            glm::vec4 center;
            glm::vec4 cameraUp;
            glm::vec4 cameraRight;
        } uboData;
    };

}


#endif //!VULPESRENDER_BILLBOARD_OBJECT_HPP