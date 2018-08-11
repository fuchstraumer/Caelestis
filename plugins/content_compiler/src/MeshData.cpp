#include "MeshData.hpp"
#include <vulkan/vulkan.h>

void MeshData::DestroyMeshData(MeshData * mesh) {

    delete[] mesh->Parts;

    for (uint32_t i = 0; i < mesh->Header.NumMaterials; ++i) {
        for (uint32_t j = 0; j < mesh->Materials[i].NumTextures; ++j) {
            delete[] mesh->Materials[i].Textures[j].Name;
        }
        delete[] mesh->Materials[i].Textures;
    }

    delete[] mesh->Materials;

    if (mesh->Header.Interleaved) {
        Vertex* vertices = reinterpret_cast<Vertex*>(mesh->Vertices);
        delete[] vertices;
    }
    else {
        UninterleavedVertexData* vertices = reinterpret_cast<UninterleavedVertexData*>(mesh->Vertices);
        delete[] vertices->Positions;
        delete[] vertices->Tangents;
        delete[] vertices->Colors;
        delete[] vertices->UV0s;
        if (vertices->UV1s) {
            delete[] vertices->UV1s;
        }
    }

    if (mesh->Header.IndexFormat == VK_INDEX_TYPE_UINT16) {
        uint16_t* indices = reinterpret_cast<uint16_t*>(mesh->Indices);
        delete[] indices;
    }
    else {
        uint32_t* indices = reinterpret_cast<uint32_t*>(mesh->Indices);
        delete[] indices;
    }

    delete mesh;
}
