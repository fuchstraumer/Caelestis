#pragma once
#ifndef VPSK_TEXTURE_DATA_HPP
#define VPSK_TEXTURE_DATA_HPP
#include <memory>

using stbi_uc = unsigned char;

namespace gli {

    class texture;

}

namespace vpsk {

    struct loaded_data_base_t {
        virtual ~loaded_data_base_t() = default;
    };

    struct stbi_image_data_t : loaded_data_base_t {
        stbi_image_data_t(const char* fname);
        ~stbi_image_data_t();
        stbi_uc* pixels;
        int x;
        int y;
        int channels;
    };

    struct gli_image_data_t : loaded_data_base_t {
        gli_image_data_t(const char* fname);
        ~gli_image_data_t();
        std::unique_ptr<gli::texture> data;
    };

    void* LoadSTB_Image(const char* fname);
    void* LoadGLI_Texture(const char* fname);

}

#endif //!VPSK_TEXTURE_DATA_HPP