#include "ObjModel.hpp"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <tinyobjloader/tiny_obj_loader.h>

struct vertex_t {
    vertex_t() : p(), uv() {}
    vertex_t(glm::vec3 _p, glm::vec2 _uv) : p(_p), uv(_uv) {}
    glm::vec3 p;
    glm::vec2 uv;
    bool operator==(const vertex_t& other) const noexcept {
        return (p == other.p) && (uv == other.uv);
    }
};

struct vertex_hash {
    size_t operator()(const vertex_t& v) const noexcept {
        return (std::hash<glm::vec3>()(v.p) ^ std::hash<glm::vec2>()(v.uv));
    }
};


LoadedObjModel::LoadedObjModel(const char * fname) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, fname)) {
        throw std::runtime_error(err);
    }

    std::unordered_map<vertex_t, uint32_t, vertex_hash> unique_vertices{};
    for (const auto& shape : shapes) {
        Indices.reserve(Indices.capacity() + shape.mesh.indices.size());
        for (const auto& index : shape.mesh.indices) {
            glm::vec3 pos {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            glm::vec2 uv{
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex_t vert(pos, uv);

            if (unique_vertices.count(vert) == 0) {
                unique_vertices[vert] = static_cast<uint32_t>(Vertices.positions.size() - 1);
                Vertices.positions.emplace_back(pos);
                Vertices.uvs.emplace_back(uv);
            }

            Indices.emplace_back(unique_vertices[vert]);
        }
    }

    Indices.shrink_to_fit();
    Vertices.positions.shrink_to_fit();
    Vertices.uvs.shrink_to_fit();

}
