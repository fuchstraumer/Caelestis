#pragma once
#ifndef TERRAIN_PLUGIN_PLANAR_MESH_HPP
#define TERRAIN_PLUGIN_PLANAR_MESH_HPP

#include "stdafx.h"
#include "mesh.hpp"

namespace vulpes {
	namespace terrain {
		class HeightNode;
	}
}

class PlanarMesh : public Mesh<mesh::Vertices, vertex_t> {
public:
    PlanarMesh(const double& side_length, const glm::ivec3& grid_pos, const glm::vec3& pos = glm::vec3(0.0f), const glm::vec3& scale = glm::vec3(1.0f), const glm::vec3& angle = glm::vec3(0.0f));
    PlanarMesh() ;
    ~PlanarMesh();
    size_t SubdivisionLevel;
    double SideLength;
    glm::ivec3 GridPos;
    void Generate(terrain::HeightNode* height_nodes);
};

#endif // !TERRAIN_PLUGIN_PLANAR_MESH_HPP
