#pragma once
#ifndef VPSK_MESH_PRIMITIVE_GENERATORS_HPP
#define VPSK_MESH_PRIMITIVE_GENERATORS_HPP
#include <memory>

namespace vpsk {

    struct MeshData;

    std::unique_ptr<MeshData> CreateBox();
    std::unique_ptr<MeshData> CreateIcosphere(const size_t detail_level);
    void GenerateTangentVectors(MeshData* mesh);

}

#endif //!VPSK_MESH_PRIMITIVE_GENERATORS_HPP
