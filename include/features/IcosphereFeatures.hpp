#pragma once
#ifndef VPSK_ICOSPHERE_FEATURES_HPP
#define VPSK_ICOSPHERE_FEATURES_HPP
#include "geometries/Icosphere.hpp"
#include "render/GraphicsPipeline.hpp"
#include <vector>
namespace vpsk {

    class IcosphereFeatures {
        IcosphereFeatures(const IcosphereFeatures&) = delete;
        IcosphereFeatures& operator=(const IcosphereFeatures&) = delete;
    public:

        IcosphereFeatures(const vpr::Device* dvc, const vpr::TransferPool* transfer_pool);
        void Init();

        void Render(const VkCommandBuffer& cmd) const;
        VkCommandBuffer Render(VkRenderPassBeginInfo pass_info, VkCommandBufferInheritanceInfo sc_info) const;
        void ResetPools();
        void AddObject(const Icosphere* object);
        const vpr::GraphicsPipelineInfo& PipelineInfo() const noexcept;
        
    private:

        void renderObjects(const VkCommandBufferBeginInfo& secondary_info) const;

        void createDescriptorPool();
        void createSetLayout();
        void createPipelineLayout();
        void createShaders();
        void setPipelineStateInfo();
        void createCommandPools();
        bool initialized = false;
        const vpr::Device* device;
        const vpr::TransferPool* transferPool;
        std::unique_ptr<vpr::GraphicsPipeline> pipeline;
        std::unique_ptr<vpr::DescriptorPool> descriptorPool;
        std::unique_ptr<vpr::DescriptorSetLayout> setLayout;
        std::unique_ptr<vpr::PipelineLayout> layout;
        std::unique_ptr<vpr::CommandPool> primaryPool;
        std::unique_ptr<vpr::CommandPool> secondaryPool;
        std::unique_ptr<vpr::ShaderModule> vert, frag;
        std::vector<const Icosphere*> objects;
        vpr::GraphicsPipelineInfo pipelineInfo;
        VkViewport viewport;
        VkRect2D scissor;
    };

}

#endif //!VPSK_ICOSPHERE_FEATURES_HPP