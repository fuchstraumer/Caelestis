#pragma once
#ifndef VPSK_STB_TEXTURE_HPP
#define VPSK_STB_TEXTURE_HPP
#include "Texture.hpp"
namespace vpsk {

    class stbTexture : public Texture {
    public:

        stbTexture(const vpr::Device* dvc, const char* fname);

    protected:

        void createFromLoadedData(void* data_ptr);
        void loadTextureDataFromFile(const char* fname) final;
        void createCopyInformation() final;
        void updateImageInfo() final;
        void updateSamplerInfo() final;
        void updateViewInfo() final;

        uint32_t width() const;
        uint32_t height() const;
        VkFormat format() const noexcept final;
        VkImageLayout finalLayout() const noexcept final;
        VkImageAspectFlags aspect() const noexcept final;
        uint32_t mipLevels() const noexcept final;
        uint32_t arrayLayers() const noexcept final;

    };

}

#endif // !VPSK_STB_TEXTURE_HPP
