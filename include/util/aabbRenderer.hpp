#pragma once 
#ifndef VULPES_VK_AABB_RENDERER_H
#define VULPES_VK_AABB_RENDERER_H

#include "AABB.hpp"
#include "ForwardDecl.hpp"
#include "resource/Buffer.hpp"
#include "render/GraphicsPipeline.hpp"
#include "resource/PipelineCache.hpp"
#include "resource/ShaderModule.hpp"
#include "resource/PipelineLayout.hpp"

namespace vpsk {

    namespace util {

        class aabbRenderer {
            aabbRenderer(const aabbRenderer&) = delete;
            aabbRenderer& operator=(const aabbRenderer&) = delete;
        public:

            aabbRenderer(const vpr::Device* dvc);
            ~aabbRenderer();

            void Clear();
            void Init(const VkRenderPass& renderpass, const glm::mat4& projection, const vpr::GraphicsPipelineInfo& pipeline_info = vpr::GraphicsPipelineInfo());
            void RecordCommands(const VkCommandBuffer& cmd, const VkCommandBufferBeginInfo& begin_info, const glm::mat4& view, const VkViewport& viewport, const VkRect2D& scissor);
            void AddAABB(const AABB& aabb);

        private:

            struct aabb_push_data {
                glm::mat4 view, projection;
            } pushData;

            void createPipelineLayout();
            void createShaders();
            void createBuffers();
            void setupGraphicsPipelineInfo(const vpr::GraphicsPipelineInfo& pipeline_info);
            void createGraphicsPipeline(const VkRenderPass& renderpass);

            void rebuildBuffers();
            void updateVBO(const VkCommandBuffer& cmd);
            bool updateRequired = false;
            std::vector<glm::vec3> vertices;
            std::vector<uint16_t> indices;

            const vpr::Device* device;

            std::unique_ptr<vpr::Buffer> vbo, ebo;
            std::unique_ptr<vpr::ShaderModule> vert, frag;
            std::unique_ptr<vpr::GraphicsPipeline> pipeline;
            std::unique_ptr<vpr::PipelineCache> pipelineCache;
            std::unique_ptr<vpr::PipelineLayout> pipelineLayout;

            VkBufferMemoryBarrier updateBarrier;
            vpr::GraphicsPipelineInfo pipelineStateInfo;
            VkGraphicsPipelineCreateInfo pipelineCreateInfo;

        };

    }

}

#endif //!VULPES_VK_AABB_RENDERER_H