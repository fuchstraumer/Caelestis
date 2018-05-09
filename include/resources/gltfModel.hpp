#pragma once
#ifndef VPSK_GLTF_MODEL_HPP
#define VPSK_GLTF_MODEL_HPP
#include "ForwardDecl.hpp"
namespace vpsk {

    class gltfTexture;

    class gltfModel {
    public:

        gltfModel(const vpr::Device* device, const char* fname);

    private:

        struct model_part_t {
            uint32_t firstIndex;
            uint32_t indexCount;
            size_t materialIndex;
        };

        void loadFromFile(const char * fname);
        const vpr::Device* device;
    };
    
} // vpsk

#endif //!VPSK_GLTF_MODEL_HPP
