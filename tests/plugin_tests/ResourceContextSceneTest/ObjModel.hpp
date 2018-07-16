#pragma once
#ifndef RESOURCE_CONTEXT_OBJ_MODEL_HPP
#define RESOURCE_CONTEXT_OBJ_MODEL_HPP
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/fwd.hpp"
#include <vector>

struct LoadedObjModel {
    LoadedObjModel(const char* fname);
    struct vertex_data_t {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uvs;
    } Vertices;
    std::vector<uint32_t> Indices;
};

#endif //!RESOURCE_CONTEXT_OBJ_MODEL_HPP
