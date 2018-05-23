#include "renderer/resources/TextureLoadFunctions.hpp"
#include "gli/gli.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
namespace vpsk {

    stbi_image_data_t::stbi_image_data_t(const char * fname) {
        pixels = stbi_load(fname, &x, &y, &channels, 4);
    }

    stbi_image_data_t::~stbi_image_data_t() {
        stbi_image_free(pixels);
    }

    void* LoadSTB_Image(const char * fname) {
        return new stbi_image_data_t(fname);
    }

    void* LoadGLI_Texture(const char * fname) {
        return new gli::texture(gli::load(fname));
    }

}