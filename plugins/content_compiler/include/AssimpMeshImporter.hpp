#pragma once
#ifndef CONTENT_COMPILER_ASSIMP_MESH_LOADER_HPP
#define CONTENT_COMPILER_ASSIMP_MESH_LOADER_HPP

struct MeshData* AssimpLoadMeshData(const char* fname, bool interleaved);

#endif //!CONTENT_COMPILER_ASSIMP_MESH_LOADER_HPP
