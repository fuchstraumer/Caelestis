#include "ObjFileImporter.hpp"
#include "MeshData.hpp"
#include "glm/vec3.hpp"
#include "glm/vec2.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#define NOMINMAX
#define TINYOBJ_LOADER_OPT_IMPLEMENTATION 
#include <tinyobjloader/experimental/tinyobj_loader_opt.h>
#include <fstream>
#include <unordered_map>

struct loaded_obj_vertex_t {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    bool operator==(const loaded_obj_vertex_t& other) const noexcept {
        return (position == other.position) && (normal == other.normal) &&
            (uv == other.uv);
    }
};

struct vertex_hash {
    size_t operator()(const loaded_obj_vertex_t& v) const noexcept {
        return (std::hash<glm::vec3>()(v.position) ^ (std::hash<glm::vec3>()(v.normal) << 32) ^ std::hash<glm::vec2>()(v.uv));
    }
};

struct LoadedObjData {
    std::vector<loaded_obj_vertex_t> vertices;
    std::vector<uint32_t> indices;
    bool hasNormals;
};

LoadedObjData loadBaseData(const char* fname) {
    tinyobj_opt::attrib_t attrib;
    std::vector<tinyobj_opt::shape_t> shapes;
    std::vector<tinyobj_opt::material_t> materials;
    std::string err;

    LoadedObjData instance_data;

    std::string file_input;
    std::ifstream input_obj(fname);
    if (!input_obj.is_open()) {
        throw std::runtime_error("Err");
    }

    std::string loaded_data{ std::istreambuf_iterator<char>(input_obj), std::istreambuf_iterator<char>() };

    if (!tinyobj_opt::parseObj(&attrib, &shapes, &materials, loaded_data.data(), loaded_data.length(), tinyobj_opt::LoadOption())) {
        throw std::runtime_error(err);
    }

    instance_data.hasNormals = !attrib.normals.empty();

    std::unordered_map<loaded_obj_vertex_t, uint32_t, vertex_hash> unique_vertices{};

    instance_data.indices.reserve(attrib.indices.size());
    instance_data.vertices.reserve(attrib.vertices.size());

    for (const auto& index : attrib.indices) {
        glm::vec3 pos {
            attrib.vertices[3 * index.vertex_index + 0],
            attrib.vertices[3 * index.vertex_index + 1],
            attrib.vertices[3 * index.vertex_index + 2]
        };

        glm::vec2 uv{
            attrib.texcoords[2 * index.texcoord_index + 0],
            1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
        };

        glm::vec3 norm(0.0f);

        if (instance_data.hasNormals) {
            norm = glm::vec3{
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2]
            };
        }

        loaded_obj_vertex_t vert{ pos, norm, uv };

        if (unique_vertices.count(vert) == 0) {
            unique_vertices[vert] = static_cast<uint32_t>(instance_data.vertices.size());
            instance_data.vertices.emplace_back(vert);
        }

        instance_data.indices.emplace_back(unique_vertices[vert]);
        
    }

    instance_data.vertices.shrink_to_fit();
    instance_data.indices.shrink_to_fit();

    return instance_data;
}

void GenerateSmoothNormals(LoadedObjData& obj_data) {
}

void GenerateTangentSpace(LoadedObjData& obj_data, std::vector<glm::vec3>& tangents, std::vector<glm::vec3>& bitangents) {

    constexpr const float angle_eps = 0.9999f;
    std::vector<bool> vert_done(obj_data.vertices.size(), false);
    constexpr const float qnan = std::numeric_limits<float>::quiet_NaN();

}

std::vector<glm::vec4> CompressBasisVectorsToQuaternions(LoadedObjData& obj_data, const std::vector<glm::vec3>& tangents, 
    const std::vector<glm::vec3>& bitangents) {

}

MeshData* LoadMeshData(const char* fname, bool interleaved) {
    
    // get positions and uvs + normals (potentially) into memory
    LoadedObjData base_data = loadBaseData(fname);

    // need normals before tangents + bitangents
    if (!base_data.hasNormals) {
        GenerateSmoothNormals(base_data);
    }

    std::vector<glm::vec3> tangents(base_data.vertices.size());
    std::vector<glm::vec3> bitangents(base_data.vertices.size());

    GenerateTangentSpace(base_data, tangents, bitangents);

    // start reformatting data: first, compress tangents into quaternions
    std::vector<glm::vec4> basis_space_quats = CompressBasisVectorsToQuaternions(base_data, tangents, bitangents);
    // Release some memory
    tangents.clear(); tangents.shrink_to_fit();
    bitangents.clear(); bitangents.shrink_to_fit();
}
