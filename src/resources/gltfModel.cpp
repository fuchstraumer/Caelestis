#include "resources/gltfModel.hpp"
#include <unordered_map>
#include <string>
#define TINYGLTF_IMPLEMENTATION
#include "tinygltf/tiny_gltf.h"
namespace vpsk {

    gltfModel::gltfModel(const vpr::Device * dvc, const char * fname) : device(dvc) {
        loadFromFile(fname);
    }

    void gltfModel::loadFromFile(const char* fname) {

    }

}
