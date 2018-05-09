#pragma once
#ifndef VPSK_GLTF_TEXTURE_HPP
#define VPSK_GLTF_TEXTURE_HPP
#include "Texture.hpp"

namespace tinygltf {
    struct Image;
}

namespace vpsk {

    class gltfTexture : public Texture {
    public:

        gltfTexture(const vpr::Device* dvc, tinygltf::Image& img, VkSampler sampler);

    protected:

        void copyDataToStaging();
        void generateMipMaps();
        void loadTextureDataFromFile(const char* fname) final;
        void createCopyInformation() final;
        VkFormat format() const;
        void updateImageInfo() final;
        void updateSamplerInfo() final;
        void updateViewInfo() final;
        VkImageLayout finalLayout() const noexcept final;
        VkImageAspectFlags aspect() const noexcept final;
        uint32_t mipLevels() const noexcept final;
        uint32_t arrayLayers() const noexcept final;

        tinygltf::Image& image;
    };

}

#endif //!VPSK_GLTF_TEXTURE_HPP