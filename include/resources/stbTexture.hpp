#pragma once
#ifndef VPSK_STB_TEXTURE_HPP
#define VPSK_STB_TEXTURE_HPP
#include "Texture.hpp"
#include "stb_image.h"
namespace vpsk {

    class stbTexture : public Texture<stbTexture> {
    public:

        stbTexture(const vpr::Device* dvc, const char* fname);

        void loadTextureDataFromFile(const char* fname);
        void createCopyInformation();
        void updateImageInfo();
        void updateSamplerInfo();
        void updateViewInfo();

        uint32_t width() const;
        uint32_t height() const;
        VkFormat format() const;
        constexpr VkImageLayout finalLayout() const;
        constexpr VkImageAspectFlags aspect() const;

        int x;
        int y;
        int channels;
        stbi_uc * pixels = nullptr;
    };

}

#endif // !VPSK_STB_TEXTURE_HPP
