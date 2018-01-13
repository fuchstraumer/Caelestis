#pragma once
#ifndef VPSK_TETHER_FEATURES_HPP
#define VPSK_TETHER_FEATURES_HPP
#include "render/GraphicsPipeline.hpp"
#include <vector>

namespace vpsk {

    class Tether;

    class TetherFeatures {
        TetherFeatures(const TetherFeatures&) = delete;
        TetherFeatures& operator=(const TetherFeatures&) = delete;
    public:

        TetherFeatures(const vpr::Device* dvc, const vpr::TransferPool* transfer_pool);
        void Init();

        VkCommandBuffer Render(VkRenderPassBeginInfo pass_info, VkCommandBufferInheritanceInfo sc_info) const;
    private:

        void setVertexPipelineData();
        void setPipelineState();
        void createSetLayout();
        void createDescriptorPool();
        void createPipelineLayout();
        void createShaders();

        std::vector<const Tether*> tethers;
        std::array<VkVertexInputAttributeDescription, 3> attr;
        std::array<VkVertexInputBindingDescription, 2> bindings;
        const vpr::Device* device;
        const vpr::TransferPool* transferPool;
        std::unique_ptr<vpr::GraphicsPipeline> pipeline;
        std::unique_ptr<vpr::DescriptorPool> descriptorPool;
        std::unique_ptr<vpr::DescriptorSetLayout> setLayout;
        std::unique_ptr<vpr::PipelineLayout> layout;
        std::unique_ptr<vpr::CommandPool> primaryPool;
        std::unique_ptr<vpr::CommandPool> secondaryPool;
        std::unique_ptr<vpr::ShaderModule> vert, frag;
        vpr::GraphicsPipelineInfo pipelineStateInfo;
        VkViewport viewport;
        VkRect2D scissor;

    };

}

#endif //!VPSK_TETHER_FEATURES_HPP