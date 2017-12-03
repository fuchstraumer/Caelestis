#pragma once
#ifndef VPSK_SUBDIVISION_SURFACE_HPP
#define VPSK_SUBDIVISION_SURFACE_HPP
#include "TriangleMesh.hpp"

namespace vpsk {

    class SubdivisionSurface : public TriangleMesh {
    public:
        SubdivisionSurface(const std::vector<vertex_t>& init_verts, const std::vector<uint32_t> init_indices, const size_t& level_to_subdivide_to);
        ~SubdivisionSurface() = default;

    private:
        struct sdFace;
        struct sdVertex : public vertex_t {
            sdFace* startFace = nullptr;
            sdVertex* child = nullptr;
            bool regular = false;
            bool boundary = false;
        };
        size_t subdivisionLevel;
    };

}

#endif //!VPSK_SUBDIVISION_SURFACE_HPP