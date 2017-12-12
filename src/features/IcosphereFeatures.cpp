#include "features/IcosphereFeatures.hpp"
#include "resource/DescriptorPool.hpp"
#include "resource/DescriptorSetLayout.hpp"
#include "resource/PipelineLayout.hpp"
#include "resource/ShaderModule.hpp"
#include "scene/BaseScene.hpp"
#include <mutex>
using namespace vpr;

namespace vpsk {

    IcosphereFeatures::IcosphereFeatures(const Device* dvc, const TransferPool* transfer_pool) : device(dvc), transferPool(transfer_pool) {}

    void IcosphereFeatures::Render(const VkCommandBuffer& cmd) {
        std::lock_guard<std::mutex> lock(std::mutex());
        for(auto obj : objects) {
            obj->Render(cmd, layout->vkHandle());
        }
    }

    void IcosphereFeatures::AddObject(const Icosphere* ico) {
        objects.push_back(ico);
    }

    void IcosphereFeatures::createSetLayout() {
        setLayout = std::make_unique<DescriptorSetLayout>(device);
        setLayout->AddDescriptorBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    }

    void IcosphereFeatures::createDescriptorPool() {
        const size_t num_textures_req = objects.size();
        descriptorPool = std::make_unique<DescriptorPool>(device);
        descriptorPool->AddResourceType(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(num_textures_req));
        descriptorPool->Create();
    }

    void IcosphereFeatures::createPipelineLayout() {
        layout = std::make_unique<PipelineLayout>(device);
        static const std::vector<VkPushConstantRange> push_constants {
            // Multiply matrices on CPU, transfer calculated MVP to shader using push constant.
            VkPushConstantRange{ VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4) }
        };
        layout->Create({ setLayout->vkHandle() }, push_constants);
    }

    void IcosphereFeatures::createShaders() {
        vert = std::make_unique<ShaderModule>(device, BaseScene::SceneConfiguration.ResourcePathPrefixStr + 
            std::string("rsrc/shaders/icosphere.vert.spv"), VK_SHADER_STAGE_VERTEX_BIT);
        frag = std::make_unique<ShaderModule>(device, BaseScene::SceneConfiguration.ResourcePathPrefixStr + 
            std::string("rsrc/shaders/icosphere.frag.spv"), VK_SHADER_STAGE_FRAGMENT_BIT);
    }
}