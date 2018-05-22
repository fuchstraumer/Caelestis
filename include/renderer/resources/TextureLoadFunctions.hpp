#pragma once
#ifndef VPSK_TEXTURE_LOADING_FUNCTIONS_HPP
#define VPSK_TEXTURE_LOADING_FUNCTIONS_HPP
#include <memory>
#include <vector>
#include <cstdint>

using stbi_uc = unsigned char;

namespace gli {

    class texture;

}

namespace vpsk {

    struct stbi_image_data_t {
        stbi_image_data_t(const char* fname);
        ~stbi_image_data_t();
        stbi_uc* pixels;
        int x;
        int y;
        int channels;
    };

    // Returns stbi_image_data_t pointer
    void* LoadSTB_Image(const char* fname);
    // Returns a gli::texture pointer
    void* LoadGLI_Texture(const char* fname);

}

#endif //!VPSK_TEXTURE_LOADING_FUNCTIONS_HPP