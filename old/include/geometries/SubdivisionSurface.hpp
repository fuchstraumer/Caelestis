#pragma once
#ifndef VPSK_SUBDIVISION_SURFACE_HPP
#define VPSK_SUBDIVISION_SURFACE_HPP
#include "TriangleMesh.hpp"
#include <memory>

namespace vpsk {

    class SubdivisionSurface : public TriangleMesh {
    public:
        SubdivisionSurface(const std::vector<vertex_t>& init_verts, const std::vector<uint32_t> init_indices, const size_t& level_to_subdivide_to);
        ~SubdivisionSurface() = default;

    private:

        struct sdFace;
        /*
            A vertice's valence is the quantity of directly adjacent vertices it has
            connected by edges. 
            Extraordinary vertices are:
            - Interior vertices with directly valence other than six
            - Boundary vertices with valence other than four.
            Thus, most sdVertices will have the regular flag set to true. A false
            value indicates the vertex is extraordinary.
        */

        constexpr size_t next_idx(const size_t& i) const noexcept {
            return (i + 1) % 3;
        }

        constexpr size_t prev_idx(const size_t& i) const noexcept {
            return (i + 2) % 3;
        }

        struct sdVertex : public vertex_t {
            sdFace* startFace = nullptr;
            sdVertex* child = nullptr;
            bool regular = false;
            bool boundary = false;
        };

        struct sdFace {

            std::array<sdVertex*, 3> v;
            std::array<sdFace*, 3> f;
            std::array<sdFace*, 4> children;
        };

        size_t subdivisionLevel;
    };

}

#endif //!VPSK_SUBDIVISION_SURFACE_HPP