
#include "resources/Material.hpp"
#include "core/LogicalDevice.hpp"
#include "command/TransferPool.hpp"
#include "resource/DescriptorSetLayout.hpp"
#include "resource/DescriptorSet.hpp"
#include "resource/Buffer.hpp"
using namespace vpr;

namespace vpsk {

    Material::Material(Material&& other) noexcept : ambient(std::move(other.ambient)), diffuse(std::move(other.diffuse)), specular(std::move(other.specular)), specularHighlight(std::move(other.specularHighlight)), bumpMap(std::move(other.bumpMap)), displacementMap(std::move(other.displacementMap)),
        alpha(std::move(other.alpha)), reflection(std::move(other.reflection)), ubo(std::move(other.ubo)), uboData(std::move(other.uboData)), setLayout(std::move(other.setLayout)), descriptorSet(std::move(other.descriptorSet)), pbrTextures(std::move(other.pbrTextures)), activeTextures(std::move(other.activeTextures)) {}

    Material & Material::operator=(Material && other) noexcept {
        ambient = std::move(other.ambient);
        diffuse = std::move(other.diffuse);
        specular = std::move(other.specular);
        specularHighlight = std::move(other.specularHighlight);
        bumpMap = std::move(other.bumpMap);
        displacementMap = std::move(other.displacementMap);
        alpha = std::move(other.alpha);
        reflection = std::move(other.reflection);
        ubo = std::move(other.ubo);
        uboData = std::move(other.uboData);
        descriptorSet = std::move(other.descriptorSet);
        pbrTextures = std::move(other.pbrTextures);
        activeTextures = std::move(other.activeTextures);
        setLayout = std::move(other.setLayout);
        return *this;
    }
   
    void Material::UploadToDevice(TransferPool* transfer_pool) {
        
        std::mutex transfer_mutex;
        std::lock_guard<std::mutex> transfer_guard(transfer_mutex);

        auto& cmd = transfer_pool->Begin();

        while(!activeTextures.empty()) {
            auto& texture_to_transfer = activeTextures.front();
            activeTextures.pop_front();
            texture_to_transfer->TransferToDevice(cmd);
        }

        ubo->CopyTo(&uboData, cmd, sizeof(uboData), 0);

        if(pbrTextures) {
            pbrTextures->ubo->CopyTo(&pbrTextures->uboData, cmd, sizeof(pbrTextures->uboData), 0);
        }

        
        transfer_pool->Submit();

    }

    void Material::BindMaterial(const VkCommandBuffer & cmd, const VkPipelineLayout& pipeline_layout) const noexcept {
        std::vector<VkDescriptorSet> descriptor_sets;
        descriptor_sets.push_back(descriptorSet->vkHandle());
        if (pbrTextures) {
            descriptor_sets.push_back(pbrTextures->descriptorSet->vkHandle());
        }
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, static_cast<uint32_t>(descriptor_sets.size()), descriptor_sets.data(), 0, nullptr);
    }

    VkDescriptorSetLayout Material::GetSetLayout() const noexcept {
        return setLayout->vkHandle();
    }

    VkDescriptorSetLayout Material::GetPbrSetLayout() const noexcept {
        if (pbrTextures) {
            return pbrTextures->setLayout->vkHandle();
        }
        else {
            return VK_NULL_HANDLE;
        }
    }

    void Material::createHashCode() {

        if (ambient != nullptr) {

        }

        if (diffuse != nullptr) {

        }

    }

}