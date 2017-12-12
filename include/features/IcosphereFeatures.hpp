#pragma once
#ifndef TRIANGLE_FEATURE_HPP
#define TRIANGLE_FEATURE_HPP
#include "geometries/Icosphere.hpp"
#include <vector>
namespace vpsk {

    class IcosphereFeatures {
        IcosphereFeatures(const IcosphereFeatures&) = delete;
        IcosphereFeatures& operator=(const IcosphereFeatures&) = delete;
    public:

        IcosphereFeatures(const vpr::Device* dvc, const vpr::TransferPool* transfer_pool);
        void Init(const glm::mat4& projection);

        void Render(const VkCommandBuffer& cmd, const VkCommandBufferBeginInfo& begin, const VkViewport& viewport, const VkRect2D& rect);
        void AddObject(const Icosphere* object);

    private:


        void createDescriptorPool();
        void createSetLayout();
        void createPipelineLayout();
        void createShaders();
        constexpr static VkVertexInputBindingDescription bindingDescription{ 0, sizeof(vertex_t), VK_VERTEX_INPUT_RATE_VERTEX };
        constexpr static VkVertexInputAttributeDescription attributeDescription{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 };
        bool initialized = false;
        const vpr::Device* device;
        const vpr::TransferPool* transferPool;
        std::unique_ptr<DescriptorPool> descriptorPool;
        std::unique_ptr<DescriptorSetLayout> setLayout;
        std::unique_ptr<PipelineLayout> layout;
        std::unique_ptr<ShaderModule> vert, frag;
        std::vector<const Icosphere*> objects;
    };

}

#endif