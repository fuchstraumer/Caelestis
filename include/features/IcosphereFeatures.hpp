#pragma once
#ifndef TRIANGLE_FEATURE_HPP
#define TRIANGLE_FEATURE_HPP
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
        void AddObject(const Icosphere* object);
        const vpr::GraphicsPipelineInfo& PipelineInfo() const noexcept;

    private:


        void createDescriptorPool();
        void createSetLayout();
        void createPipelineLayout();
        void createShaders();
        void setPipelineStateInfo();
        bool initialized = false;
        const vpr::Device* device;
        const vpr::TransferPool* transferPool;
        std::unique_ptr<DescriptorPool> descriptorPool;
        std::unique_ptr<DescriptorSetLayout> setLayout;
        std::unique_ptr<PipelineLayout> layout;
        std::unique_ptr<ShaderModule> vert, frag;
        std::vector<const Icosphere*> objects;
        vpr::GraphicsPipelineInfo pipelineInfo;
    };

}

#endif