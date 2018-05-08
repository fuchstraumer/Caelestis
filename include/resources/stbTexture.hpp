#pragma once
#ifndef VPSK_STB_TEXTURE_HPP
#define VPSK_STB_TEXTURE_HPP
#include "Texture.hpp"
namespace vpsk {

    class stbTexture : public Texture {
    public:

        stbTexture(const vpr::Device* dvc, const char* fname);

        void loadTextureDataFromFile(const char* fname);
        void createCopyInformation();
        void updateImageInfo();
        void updateSamplerInfo();
        void updateViewInfo();

        uint32_t width() const;
        uint32_t height() const;
        VkFormat format() const noexcept final;
        virtual VkImageLayout finalLayout() const noexcept final;
        virtual VkImageAspectFlags aspect() const noexcept final;
        virtual uint32_t mipLevels() const final;
        virtual uint32_t arrayLayers() const final;

        int x;
        int y;
        int channels;

    };

}

#endif // !VPSK_STB_TEXTURE_HPP
